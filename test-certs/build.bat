openssl req -days 3650 -nodes -new -x509 -keyout ca.key -out ca.crt -config openssl.cnf

openssl dhparam -out dh1024.pem 1024

echo 00 > serial
openssl req -days 3650 -nodes -new -keyout server.key -out server.csr -config openssl.cnf
openssl ca -days 3650 -out server.crt -in server.csr -extensions server -config openssl.cnf

openssl req -days 3650 -nodes -new -keyout client.key -out client.csr -config openssl.cnf
openssl ca -days 3650 -out client.crt -in client.csr -config openssl.cnf
