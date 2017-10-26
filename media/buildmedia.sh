#!/bin/bash
rm -rf media.tar.gz;
rm -rf media.zip;
tar -cvzf media.tar.gz . --exclude getmedia.sh --exclude buildmedia.sh --exclude README --exclude media.tar --exclude .svn; 
zip -r media.zip * -x media.tar.gz -x getmedia.sh -x buildmedia.sh -x .svn -x README;
rm -rf .ftpscript;
echo open -u objective3d ftpperso.free.fr >> .ftpscript;
echo cd \"release/medias/\" >> .ftpscript;
echo put "media.tar.gz" >> .ftpscript;
echo put "media.zip" >> .ftpscript;
lftp -f .ftpscript;
#scp -P2222 media.tar.gz o3d@gemelos.ww7.be:"html/release/medias/media.tar.gz"
#scp -P2222 media.zip o3d@gemelos.ww7.be:"html/release/medias/media.zip"
rm -rf media.tar.gz;
rm -rf media.zip;
rm -rf .ftpscript;
