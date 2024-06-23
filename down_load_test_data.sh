#!/bin/bash

if [ ! -d "test_data/glib" ]; then
    mkdir -p test_data/glib
fi

cd test_data/glib

wget https://download.gnome.org/sources/glib/2.70/glib-2.70.0.tar.xz
wget https://download.gnome.org/sources/glib/2.70/glib-2.70.1.tar.xz
wget https://download.gnome.org/sources/glib/2.70/glib-2.70.2.tar.xz
wget https://download.gnome.org/sources/glib/2.70/glib-2.70.3.tar.xz
wget https://download.gnome.org/sources/glib/2.70/glib-2.70.4.tar.xz
wget https://download.gnome.org/sources/glib/2.70/glib-2.70.5.tar.xz

wget https://download.gnome.org/sources/glib/2.71/glib-2.71.0.tar.xz
wget https://download.gnome.org/sources/glib/2.71/glib-2.71.1.tar.xz
wget https://download.gnome.org/sources/glib/2.71/glib-2.71.2.tar.xz
wget https://download.gnome.org/sources/glib/2.71/glib-2.71.3.tar.xz

wget https://download.gnome.org/sources/glib/2.72/glib-2.72.0.tar.xz
wget https://download.gnome.org/sources/glib/2.72/glib-2.72.1.tar.xz
wget https://download.gnome.org/sources/glib/2.72/glib-2.72.2.tar.xz
wget https://download.gnome.org/sources/glib/2.72/glib-2.72.3.tar.xz
wget https://download.gnome.org/sources/glib/2.72/glib-2.72.4.tar.xz

wget https://download.gnome.org/sources/glib/2.73/glib-2.73.0.tar.xz
wget https://download.gnome.org/sources/glib/2.73/glib-2.73.1.tar.xz
wget https://download.gnome.org/sources/glib/2.73/glib-2.73.2.tar.xz
wget https://download.gnome.org/sources/glib/2.73/glib-2.73.3.tar.xz

xz -d *.xz
rm -rf *.xz