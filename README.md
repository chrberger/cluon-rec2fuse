# cluon-rec2fuse

`cluon-rec2fuse` is a microservice for [libcluon](https://github.com/chrberger/libcluon)-based [OD4Sessions](https://github.com/chalmers-revere/opendlv) to _mount_ existing recordings (like, for instance, dumps from tool [`cluon-OD4toStdout`](https://github.com/chrberger/libcluon/blob/master/libcluon/tools/cluon-OD4toStdout.cpp)) into a folder while mapping the contained messages as separate .csv files.

[![License](https://img.shields.io/badge/license-GPL--3-blue.svg)](https://raw.githubusercontent.com/chrberger/libcluon/master/LICENSE) [![x86_64](https://img.shields.io/badge/platform-x86_64-blue.svg)](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

## Table of Contents
* [Features](#features)
* [Dependencies](#dependencies)
* [Usage](#usage)
* [License](#license)

## Features
* Dynamically maps [`Envelope`](https://github.com/chrberger/libcluon/blob/master/libcluon/resources/cluonDataStructures.odvd#L23-L30) and their contained messages as .csv files using [`libfuse`](https://github.com/libfuse/libfuse)
* Including `sent`, `received`, and `sample time point` time stamps
* Dynamically maps [`Envelope`](https://github.com/chrberger/libcluon/blob/master/libcluon/resources/cluonDataStructures.odvd#L23-L30) and their contained messages as .csv files using [`libfuse`](https://github.com/libfuse/libfuse)
* Available as Docker images for [x86_64](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

## Dependencies
* [`libfuse`](https://github.com/libfuse/libfuse)
* A C++14-compliant compiler

This project as ships the following dependency as part of the source distribution:

* [libcluon](https://github.com/chrberger/libcluon) - [![License: GPLv3](https://img.shields.io/badge/license-GPL--3-blue.svg
)](https://www.gnu.org/licenses/gpl-3.0.txt)

## Usage
This microservice is created automatically on changes to this repository via Docker's public registry for [x86_64](https://hub.docker.com/r/chrberger/cluon-proto-amd64/tags/)

This microservice can be used during data post-processing for [libcluon](https://github.com/chrberger/libcluon)-based applications to easily access the contents of a .rec file. A .rec-file is simply a dump of all exchanged [`Envelope`](https://github.com/chrberger/libcluon/blob/master/libcluon/resources/cluonDataStructures.odvd#L23-L30) that could be created by using the tool [`cluon-OD4toStdout`](https://github.com/chrberger/libcluon/blob/master/libcluon/tools/cluon-OD4toStdout.cpp) redirecting its output into a file.

Let's assume you have a message specification like the [OpenDLV Standard Message Set](https://github.com/chalmers-revere/opendlv.standard-message-set) named `opendlv.odvd` that you would like to use for mapping the content from the file `myrecording.rec` into a folder `mnt` as .csv files. Then, you can use this microservice as follows:

```
docker run --rm -ti -v $PWD/myrecording.rec:/opt/input.rec \
                    -v $PWD/opendlv.odvd:/opt/odvd \
                    -v $PWD/mnt:/opt/output:shared \
                    --cap-add SYS_ADMIN \
                    --cap-add MKNOD \
                    --security-opt apparmor:unconfined \
                    --device=/dev/fuse \
                    -v /etc/passwd:/etc/passwd:ro \
                    -v /etc/group:/etc/group \
                    chrberger/cluon-rec2fuse-amd64:v0.0.90 \
                    /bin/sh -c "chown $UID:$UID /opt/output && \
                    su -s /bin/sh $USER -c 'cluon-rec2fuse --rec=/opt/input.rec --odvd=/opt/odvd -f /opt/output' \
                    && tail -f /dev/null"
```

When you have finished working with the mapped files in the `mnt` folder, stop the microservice by pressing Ctrl-C. Afterwards, you need to unmount the folder `mnt` as follows:

```
fusermount -u mnt
```

You can watch the usage of this microservice here:
[![asciicast](https://asciinema.org/a/tMLc9lvmnTKlcwSHSIuepF4It.png)](https://asciinema.org/a/tMLc9lvmnTKlcwSHSIuepF4It?autoplay=1)

## License

* This project is released under the terms of the GNU GPLv3 License
