@echo off

echo d > input.txt
echo encrypted.txt >> input.txt
echo decrypted.txt >> input.txt
echo 3 >> input.txt
echo mypassword >> input.txt

encryptor < input.txt

del input.txt
