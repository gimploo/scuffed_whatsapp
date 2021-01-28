# scuffed whatsapp (tui)

A fun little project for me to learn about socket programming, multithreading using the pthreads api and how to implement all that in c. The goal is to have it function more or less like tui version of whatsapp i.e private one to one connections along with a group chat functionality to be the most important aspects.

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
    [x] Private one to one connection 
    [x] Implement a Friends list
    [x] Implement a Groups list
    [ ] Have user be able to make mutiple groups with different names
    [ ] Have groups function more like chatrooms than one way broadcasts
    [ ] Delete groups
    [ ] Implement thread pools on server side



