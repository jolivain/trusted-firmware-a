get_doc() {
    grep -r "cmake:variable:: $1$" -A 10000 | sed '/#]/Q' | sed 's/.*   //g' | sed 's/.*-$/ /g' | sed 's/.*-\.\. _/.. _/g' |tail -n +3
}

filename="docs/cmake/manual/variable/config/$1.rst" 
namelength=${#1}
touch $filename
echo $1 >> $filename

for (( n = 0; n < $namelength; n++ ))
do 
echo -n "=" >> $filename
done
echo -e '\n' >> $filename
echo -e ".. default-domain:: cmake\n" >> $filename
echo -e ".. variable:: ${1}\n" >> $filename

# get_doc $1 | sed 's/^/    /' >> $filename
get_doc $1 >> $filename
echo "
--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*" >> $filename
