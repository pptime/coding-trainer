#!/usr/bin/bash

dfs(){
	echo $1;
	if [ -d "$1" ];then
		for file in $1/*;
			do dfs ${file};
		done
	fi
}

randomdfs() {
	if [ -f "$1" ]; then
		echo $1;
		gedit $1;
	fi
	if [ -d "$1" ]; then
		local file_list=($1/*);
		local file_list_length=${#file_list[@]};
		randomdfs ${file_list[$(($RANDOM % $file_list_length))]}
	fi
}




randomdfs "problems"
