#!/bin/bash

# Updating texpp trails for scwise python part

current_dir=`pwd`"/../"
opt_dir="/opt/texpp/"
build_dir=${current_dir}"/build/"

if [ ! -d "${build_dir}" ];
then
    echo "Can not find build dir. Might be you didn't build."
    exit 1;
fi

if [ ! -d "${opt_dir}" ];
then
    echo "Can not find ${opt_dir}. It doesn't exists or wrond path"
    exit 1;
fi

cd ${build_dir}

so_files=( "_chrefliterals.so"  "libtexpp.so"  "texpy.so" )

for i in "${so_files[@]}"
do
    search_term="*${i}"
    file_to_copy=`find -wholename ${search_term}`
    if [ -z "${file_to_copy}" ];
    then
        echo "No compiled ${i} which can leads to a problems."
        echo "Wrong search term ${search_term}"
        exit 1;
    else
        echo "${i} exists...";
        `rm ${opt_dir}/${i}`
        `cp ${file_to_copy} ${opt_dir}/${i}`
    fi
done

echo "Updated..."

cd ${current_dir}/scripts