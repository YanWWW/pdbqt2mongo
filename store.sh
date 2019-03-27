#!/bin/sh
# 加压当前文件夹下的每一个.gz压缩包
# 使用pdbqt2mongo将解压的文档存入mongoDB

# 等号旁边不能有空白
count="1"
post=".gz"
mkdir ./tmp

# 通配符*表示任意数量字符
for file in *"$post"
do

  # % 在makefile中相当于系统的通配符*
  # 在bash中%是函数，删除最末尾的字符
  name="${file%$post}"
  gunzip -c "$file" > ./tmp/"$name"

  # -n 不换行的意思
  echo -n "$count" :
  let "count=count+1"
  ./pdbqt2mongo ./tmp/"$name"
  rm ./tmp/"$name"
done

  # 匹配中括号中任意一个字符。
  # 例如：[abc] 代表一定匹配一个字符，或者是a或者是b，或者是c
  # [ -z STRING ] “STRING” 的长度为零则为真。
  # 正则表达：https://blog.csdn.net/qq_37699336/article/details/80879531
if [ -z "$(ls -A ./tmp/)" ]; then
  rm -rf ./tmp
fi
