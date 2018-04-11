# cluon-rec2fuse

`cluon-rec2fuse` is a microservice for [libcluon](https://github.com/chrberger/libcluon)-based [OD4Sessions](https://github.com/chalmers-revere/opendlv) to _mount_ existing recordings (like, for instance, dumps from tool [`cluon-OD4toStdout`](https://github.com/chrberger/libcluon/blob/master/libcluon/tools/cluon-OD4toStdout.cpp)) into a folder by mapping the contained messages as .csv files.

[![License](https://img.shields.io/badge/license-GPL--3-blue.svg)](https://raw.githubusercontent.com/chrberger/libcluon/master/LICENSE) [![x86_64](https://img.shields.io/badge/platform-x86_64-blue.svg)](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

## Table of Contents
* [Features](#features)
* [Dependencies](#dependencies)
* [Usage](#usage)
* [License](#license)

## Features
* Written in highly portable and high quality C++14
* Dynamically maps [`Envelope`](https://github.com/chrberger/libcluon/blob/master/libcluon/resources/cluonDataStructures.odvd#L23-L30) and their contained messages as .csv files using [`libfuse`](https://github.com/libfuse/libfuse)
* Available as Docker images for [x86_64](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

## Dependencies
No dependencies! You just need a C++14-compliant compiler to compile this
project as it ships its dependencies as part of the source distribution:

* [libcluon](https://github.com/chrberger/libcluon) - [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)

## Usage
This microservice is created automatically on changes to this repository via Docker's public registry for [x86_64](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

This microservice is supposed to be used during data post-processing for [libcluon](https://github.com/chrberger/libcluon)-based applications easily access the contents of a .rec file that could be created by using the tool [`cluon-OD4toStdout`](https://github.com/chrberger/libcluon/blob/master/libcluon/tools/cluon-OD4toStdout.cpp) redirecting its output into a file.

Let's assuming you have a message specification like the [OpenDLV Standard Message Set](https://github.com/chalmers-revere/opendlv.standard-message-set) that you would like to use for mapping the content from the .rec file into a folder as .csv files. Then, you can use this microservice as follows:


## License

* This project is released under the terms of the GNU GPLv3 License
