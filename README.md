# scuffed whatsapp

A fun little project for me to learn about socket programming and how to implement in c. The goal is to have it function more or less like whatsapp i.e private one to one connections along with a group chat functionality to be most important.

## Usage
1. Build the project 

        make 

2. Run the server

        ./server

3. Run the client
    
   If you have the server hosted on a server else where pass that ip to the client as an argument

        ./client <ip_address>

#### TODO:
    [x] Minimal setup client to server connection
    [x] Multi threading  
    [x] Unique username setup logic
    [x] Group chat functionality
    [ ] Private one to one connection 
    [ ] Server and client optimizations



