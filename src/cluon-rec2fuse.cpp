/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <cstdint>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

// Maps of container-ID & sender-stamp.
std::map<std::string, std::string> mapOfFilenames;
std::map<std::string, std::string> mapOfEntries;
std::map<std::string, uint32_t> mapOfEntrySizes;

static int getattr_callback(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    for (auto entry : mapOfEntries) {
        std::stringstream sstr;
        sstr << "/" << mapOfFilenames[entry.first] << ".csv";
        const std::string FQDN = sstr.str();
        if (strcmp(path, FQDN.c_str()) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            stbuf->st_size = mapOfEntrySizes[entry.first];
            return 0;
        }
    }

    return -ENOENT;
}

static int readdir_callback(const char */*path*/, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (auto entry : mapOfEntries) {
        std::stringstream sstr;
        sstr << "/" << mapOfFilenames[entry.first] << ".csv";
        const std::string FQDN = sstr.str();
        filler(buf, FQDN.c_str()+1, NULL, 0); // Omit leading '/'
    }

    return 0;
}

static int open_callback(const char */*path*/, struct fuse_file_info */*fi*/) {
    return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info */*fi*/) {
    for (auto entry : mapOfEntries) {
        std::stringstream sstr;
        sstr << "/" << mapOfFilenames[entry.first] << ".csv";
        const std::string FQDN = sstr.str();

        if (strcmp(path, FQDN.c_str()) == 0) {
            size_t len = mapOfEntrySizes[entry.first];
            if (offset >= static_cast<int32_t>(len)) {
                return 0;
            }

            const std::string filecontent = mapOfEntries[entry.first];

            if (offset + size > len) {
                memcpy(buf, filecontent.c_str() + offset, len - offset);
                return len - offset;
            }

            memcpy(buf, filecontent.c_str() + offset, size);
            return size;
        }
    }

    return -ENOENT;
}


static struct fuse_operations rec2fuse_operations;

////////////////////////////////////////////////////////////////////////////////

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{0};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("rec")) || (0 == commandlineArguments.count("odvd")) ) {
        std::cerr << argv[0] << " maps the content from a given .rec file using a provided .odvd message specification into a folder using FUSE." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --rec=<Recording from an OD4Session> --odvd=<ODVD Message Specification> -f </my/directory>" << std::endl;
        std::cerr << "Example: " << argv[0] << " --rec=myRecording.rec --odvd=myMessage -f ~/data" << std::endl;
        retCode = 1;
    } else {
        cluon::MessageParser mp;
        std::pair<std::vector<cluon::MetaMessage>, cluon::MessageParser::MessageParserErrorCodes> messageParserResult;
        {
            std::ifstream fin(commandlineArguments["odvd"], std::ios::in|std::ios::binary);
            if (fin.good()) {
                std::string input(static_cast<std::stringstream const&>(std::stringstream() << fin.rdbuf()).str()); // NOLINT
                fin.close();
                messageParserResult = mp.parse(input);
                std::clog << "Found " << messageParserResult.first.size() << " messages." << std::endl;
            }
            else {
                std::cerr << argv[0] << ": Message specification '" << commandlineArguments["odvd"] << "' not found." << std::endl;
                return retCode;
            }
        }

        std::fstream fin(commandlineArguments["rec"], std::ios::in|std::ios::binary);
        if (fin.good()) {
            fin.close();

            std::map<int32_t, cluon::MetaMessage> scope;
            for (const auto &e : messageParserResult.first) { scope[e.messageIdentifier()] = e; }

            constexpr bool AUTOREWIND{false};
            constexpr bool THREADING{false};
            cluon::Player player(commandlineArguments["rec"], AUTOREWIND, THREADING);

            while (player.hasMoreData()) {
                auto next = player.getNextEnvelopeToBeReplayed();
                if (next.first) {
                    cluon::data::Envelope env{std::move(next.second)};
                    if (scope.count(env.dataType()) > 0) {
                        cluon::FromProtoVisitor protoDecoder;
                        std::stringstream sstr(env.serializedData());
                        protoDecoder.decodeFrom(sstr);

                        cluon::MetaMessage m = scope[env.dataType()];
                        cluon::GenericMessage gm;
                        gm.createFrom(m, messageParserResult.first);
                        gm.accept(protoDecoder);

                        std::stringstream sstrKey;
                        sstrKey << env.dataType() << "/" << env.senderStamp();
                        const std::string KEY = sstrKey.str();

                        std::stringstream sstrFilename;
                        sstrFilename << m.messageName() << "-" << env.senderStamp();
                        const std::string _FILENAME = sstrFilename.str();

                        mapOfFilenames[KEY] = _FILENAME;
                        if (mapOfEntries.count(KEY) > 0) {
                            cluon::ToCSVVisitor csv(';', false);
                            gm.accept(csv);
                            mapOfEntries[KEY] +=  csv.csv();
                        }
                        else {
                            cluon::ToCSVVisitor csv(';', true);
                            gm.accept(csv);
                            mapOfEntries[KEY] +=  csv.csv();
                        }
                        mapOfEntrySizes[KEY] = mapOfEntries[KEY].size();
                    }
                }
            }

            // Setting up FUSE:
            rec2fuse_operations.getattr = getattr_callback;
            rec2fuse_operations.open = open_callback;
            rec2fuse_operations.read = read_callback;
            rec2fuse_operations.readdir = readdir_callback;

            // Remove our args before calling FUSE.
            std::vector<std::string> args;
            for (int32_t i = 0; i < argc; i++) {
                if (i > 2) {
                    args.push_back(std::string(argv[i]));
                }
            }

            char **argv2 = new char*[args.size()];
            for (uint32_t i = 0; i < args.size(); i++) {
                argv2[i] = const_cast<char*>(args[i].c_str());
            }

            return fuse_main(args.size(), argv2, &rec2fuse_operations, NULL);
        }
        else {
            std::cerr << argv[0] << ": Recording '" << commandlineArguments["rec"] << "' not found." << std::endl;
        }
    }
    return retCode;
}
