"c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\makecert.exe" ^
-n "CN=Warcis Gaming,OU=Game,O=Warcis,L=Moscow,S=EU,C=RU,E=warcis@warcis.com" ^
-r ^
-ss CA ^
-pe ^
-a sha512 ^
-len 4096 ^
-sky signature ^
-cy end ^
-sv WarcisRoot.pvk WarcisRoot.cer

"c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\pvk2pfx.exe" ^
-pvk WarcisRoot.pvk ^
-spc WarcisRoot.cer ^
-pfx WarcisRoot.pfx ^
-po Venmade1Warcis2Cert3

certutil.exe -f -addstore Root WarcisRoot.cer