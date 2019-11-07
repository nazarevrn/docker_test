FROM debian:bionic

RUN mkdir /root/distr
RUN cd /root/distr

COPY httpd-2.2.3.tar.gz /var/www/httpd-2.2.3.tar.gz
COPY php-5.1.6.tar.gz /var/www/php-5.1.6.tar.gz
COPY mysql-5.0.95.tar.gz /var/www/mysql-5.0.95.tar.gz


RUN apt-get update

RUN apt-get install -y build-essential flex libxml2-dev mysql-server libmysqlclient-dev


# RUN   cd var/www/ \
        tar -xzf httpd-2.2.3.tar.gz \
        cd httpd-2.2.3 \
#       ./configure --enable-so \
#       make \
#       make install \

    cd var/www/ \
    tar -xzf mysql-5.0.95.tar.gz \
    cd mysql-5.0.95.tar.gz \

    ./configure --enable-profiling --enable-community-features --prefix=/opt/mysql-5.0.95/ --enable-local-infile \
     --with-mysqld-user=mysql  --with-big-tables \
     --with-plugins=partition,blackhole,federated,heap,innodb_plugin --without-docs \
     --with-named-curses-libs=/opt/ncurses/lib/libncurses.a


    cd ../php-5-1-6
    ./configure --with-apxs2=/usr/local/apache2/bin/apxs --with-mysqli
    make
    make install



#RUN echo " deb http://mariadb.cu.be//repo/10.0/debian wheezy main
#deb-src http://mariadb.cu.be//repo/10.0/debian wheezy main" > /etc/apt/sources.list.d/mariadb.list


#docker run -i -t -v $PWD:/root/distr:rw ubuntu:bionic /bin/bash

docker run -i -t -v $PWD:/root/distr:rw i386/ubuntu:bionic /bin/bash

wget http://dev.mysql.com/get/Downloads/MySQL-5.0.95/mysql-5.0.95.tar.gz/from/http://mysql.he.net/



apt-get update
apt-get install build-essential -y

