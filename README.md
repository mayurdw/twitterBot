# Twitter Bot

I want to be able to get old posts from my blog and share them on Twitter in a predefined text. I have a WordPress blog which only gives me posts in RSS format so I needed my bot to be able to unpack RSS files, extract blog post Title & URL. The extracted Blog Post would be stored in an internal database file and updated regularly with new posts. 

This project as it stands does 2/3 things it was designed to do:
1. It downloads RSS file from my blog
2. It updates an internal Database with new blog posts. 

The plan to share things on Twitter automatically is scrapped/deferred for now. 

## Installation & Execution:

1. You will need WSL.exe installed on your machine
2. After Ubuntu is installed, install build-essentials on it
3. Install LibCurl dev open SSL 
4. Install LibXML2 dev
5. Install CMake
