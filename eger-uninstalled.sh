# generated by configure / remove this line to disable regeneration
prefix="/usr/local"
exec_prefix="${prefix}"
bindir="${exec_prefix}/bin"
libdir="/home/imilyakov/service-proxy/eger/.libs"
datarootdir="${prefix}/share"
datadir="${datarootdir}"
sysconfdir="${prefix}/etc"
includedir="/home/imilyakov/service-proxy/eger/."
package="eger"
suffix=""

for option; do case "$option" in --list-all|--name) echo  "eger"
;; --help) pkg-config --help ; echo Buildscript Of "eger Library"
;; --modversion|--version) echo "0.1"
;; --requires) echo : ""
;; --libs) echo -L${libdir} "" "-leger"
       :
;; --cflags) echo -I${includedir} ""
       :
;; --variable=*) eval echo '$'`echo $option | sed -e 's/.*=//'`
;; --uninstalled) exit 0 
;; *) ;; esac done