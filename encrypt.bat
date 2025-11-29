@echo off

echo e > input.txt
echo test.txt >> input.txt
echo encrypted.txt >> input.txt
echo 3 >> input.txt
echo mypassword >> input.txt

encryptor < input.txt

del input.txt
