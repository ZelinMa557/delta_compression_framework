#!/bin/bash
create_if_not_exist() {
    if [ ! -d $1 ]; then
        mkdir -p $1
    fi
}
create_if_not_exist test_data/glib

cd test_data/glib
for i in {61..80}
do
    wget https://download.gnome.org/sources/glib/2.$i/glib-2.$i.0.tar.xz
done


xz -d *.xz
rm -rf *.xz

cd ../..
create_if_not_exist test_data/linux

cd test_data/linux
for i in {1..5}
do
    for (( j=1; j<=10; j+=3))
    do
        wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.$i.$j.tar.xz
    done
done
xz -d *.xz
rm -rf *.xz
cd ../..

create_if_not_exist test_data/postgres
cd test_data/postgres
postgre_versions=(12.0 12.4 12.8 12.12 12.16
                  13.0 13.4 13.8 13.12 13.16
                  14.0 14.4 14.8 14.12 14.13
                  15.0 15.4 15.8 16.0 16.4)
for postgre_version in "${postgre_versions[@]}"
do
  wget https://ftp.postgresql.org/pub/source/v$postgre_version/postgresql-$postgre_version.tar.gz
  gzip -d postgresql-$postgre_version.tar.gz
done

cd ../..

create_if_not_exist test_data/gcc
cd test_data/gcc
gcc_versions=(12.1.0 12.2.0 12.3.0 12.4.0
              13.1.0 13.2.0 13.3.0 14.1.0)
for gcc_version in "${gcc_versions[@]}"
do
    wget https://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-$gcc_version/gcc-$gcc_version.tar.xz
done
xz -d *.xz
rm -rf *.xz