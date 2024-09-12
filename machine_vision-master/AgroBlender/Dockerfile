FROM nvidia/cuda:10.1-cudnn7-devel

ENV DEBIAN_FRONTEND noninteractive

# # prepering enviroment to compile blender
RUN apt-get update && \
    apt-get install sudo && \
    apt-get install -y software-properties-common

RUN add-apt-repository ppa:deadsnakes/ppa && \
    apt-get update && \
    apt-get install -y python3.5

RUN apt-get install -y git

WORKDIR /home/

# WORKDIR /home
RUN mkdir TriffidBlender
WORKDIR /home/TriffidBlender

# # Before building container download blender repository to your hosting machine
# # using commands: 
# # git clone http://gitlab.whitehill.eu/TRIFFID/blender.git
# # pushd blender
# # git submodule init
# # git submodule update
# # popd

COPY  blender/ blender/

RUN ./blender/build_files/build_environment/install_deps.sh --install=$PWD/deps --source=$PWD/deps-sources

RUN mkdir build_linux
WORKDIR /home/TriffidBlender/build_linux

RUN apt-get install libopenjp2-7-dev

RUN wget https://bootstrap.pypa.io/get-pip.py && \
 	python3.5 get-pip.py --user && \
 	rm get-pip.py

RUN /root/.local/bin/pip3.5 install requests
RUN ln -s /usr/local/lib/python3.5/dist-packages /usr/lib/python3.5/dist-packages

RUN cmake \
  -D CMAKE_BUILD_TYPE=Debug \
  -D WITH_CODEC_SNDFILE=ON \
  -D WITH_OPENCOLORIO=ON \
  -D OPENCOLORIO_ROOT_DIR=$PWD/../deps/ocio \
  -D WITH_OPENIMAGEIO=ON \
  -D OPENIMAGEIO_ROOT_DIR=$PWD/../deps/oiio \
  -D WITH_CYCLES_OSL=ON \
  -D WITH_LLVM=ON \
  -D LLVM_ROOT_DIR=$PWD/../deps/llvm \
  -D OSL_ROOT_DIR=$PWD/../deps/osl \
  -D WITH_OPENSUBDIV=ON \
  -D OPENSUBDIV_ROOT_DIR=$PWD/../deps/osd \
  -D WITH_OPENVDB=ON \
  -D WITH_OPENVDB_BLOSC=ON \
  -D OPENVDB_ROOT_DIR=$PWD/../deps/openvdb \
  -D WITH_ALEMBIC=ON \
  -D ALEMBIC_ROOT_DIR=$PWD/../deps/alembic \
  -D WITH_CODEC_FFMPEG=ON \
  -D FFMPEG_LIBRARIES='avformat;avcodec;avutil;avdevice;swscale;swresample;lzma;rt;theora;theoradec;theoraenc;vorbis;vorbisenc;vorbisfile;ogg;x264;openjp2' ../blender/

RUN make -k -j$(nproc) ; make install

RUN apt-get install curl

WORKDIR /home/TriffidBlender/build_linux/bin/2.79/python/bin

RUN curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py

RUN ./python3.5m ./get-pip.py

RUN ./python3.5m -m pip install asciitree
RUN ./python3.5m -m pip install rpyc==4.1.0
WORKDIR /home/TriffidBlender

COPY TomatoGenerator/ TomatoGenerator/
WORKDIR /home/TriffidBlender/TomatoGenerator/
RUN /bin/bash -c "source ./pythonpath.source"


