FROM debian:wheezy


RUN mkdir /home/docker

COPY httpd-2.2.3 /home/docker/httpd-2.2.3

COPY php-5.1.6 /home/docker/php-5.1.6

RUN echo "deb http://archive.debian.org/debian wheezy main \
deb http://archive.debian.org/debian-archive/debian-security/ wheezy updates/main"  > /etc/apt/sources.list

RUN apt-get update

RUN apt-get install -y \
    build-essential \
    flex \
    libxml2-dev \

# RUN cd httpd-2.2.3 \
#     ./configure --enable-so \
#     make \
#     make install \



#RUN echo " deb http://mariadb.cu.be//repo/10.0/debian wheezy main
#deb-src http://mariadb.cu.be//repo/10.0/debian wheezy main" > /etc/apt/sources.list.d/mariadb.list