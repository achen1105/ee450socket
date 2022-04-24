a) Anita Chen

b) achen999 4361009421

c) EE 450 Spring 2022 Socket Project
Phases 1 through 3 are complete (all parts of the project are complete except the optional Phase 4 stats command).

d) 
README.md - project information
makefile - the makefile for the project (includes 'make all')
serverM.cpp - runs server M ('./serverM')
serverA.cpp - runs server A ('./serverA')
serverB.cpp - runs server B ('./serverB')
serverC.cpp - runs server C ('./serverC')
clientA.cpp - runs client A ('./clientA <username>')
clientB.cpp - runs client B ('./clientB <username>')
block1.txt - provided first block for serverA (not included)
block2.txt - provided second block for serverB (not included)
block3.txt - provided third block for serverC (not included)
alichain.txt - list generated after running TXLIST command (not included)

e) 
SERVER M
Booting up: The main server is up and running.
CHECKWALLET: The main server received input='<username>' from the client using TCP over port <portnumber>.
TXCOINS: The main server received from <username1> to transfer <amount> coins to <username2> using TCP over port <portnumber>.
Checking balance: The main server sent a request to server <i>.
Received balance: The main server received transactions from Server <i> using UDP over port
<portnumber>.
Sent balance: The main server sent the current balance to client <j>.
Checking transfer: The main server sent a request to server <i>.
Received transfer status: The main server received the feedback from server <i> using UDP over port <portnumber>.
Sent transfer result: The main server sent the result of the transaction to client <j>.
TXLIST: A sorted list request from client <i> has been received.
Generated TXLIST: The sorted file is up and ready.
Generated stats: The main server sent the statistics to client <i>.

SERVER A, B, C
Booting up: The Server<i> is up and running using UDP on port <portnumber>.
Received request from Server M: The Server<i> received a request from the Main Server.
Sent response to Server M: The Server<i> finished sending the response to the Main Server.

CLIENT A, B
Booting up: The client <i> is up and running.
Send CHECKWALLET: <username> sent a balance enquiry request to the main server.
Receive balance: The current balance of <username> is: <amount> alicoins.
Receive balance (not in network): Unable to proceed with the request as <username> is not part of the network.
Send TXCOINS: <username1> has requested to transfer <amount> coins to <username2>.
Receive transfer result (successful): <username1> successfully transferred <amount> alicoins to <username2>.
The current balance of <username1> is: <amount_balance> alicoins.
Receive transfer result (insufficient balance): <username1> was unable to transfer <amount> alicoins to <username2> because of insufficient balance.
The current balance of <username1> is: <amount_balance> alicoins.
Receive transfer result (one not in network): Unable to proceed with the transaction as <username1/2> is not part of the network.
Receive transfer result (both not in network): Unable to proceed with the transaction as <username1> and <username2> are not part of the network.
Send TXLIST: Client <i> sent a sorted list request to the main server.
Confirm list: Sorted list was generated.
Send stats: <username> sent a statistics balance enquiry request to the main server.
Receive stats: <username>'s statistics are the following: Rank--Username--NumofTransacions--Total
<ranking>                        

g) After Client A ends, Client B must go next.  If Client A or Client B is started twice in a row instead of alternating, Server M stops listening for messages.

h) I modified code from the following:
1) insertion sort: https://www.geeksforgeeks.org/insertion-sort/
Found in void insertionSort(string arr[], int n) from serverM
2) 6.1 - 6.3 TCP and UDP client and server syntax: https://beej.us/guide/bgnet/html/
#client-server-background
Found in structure of server M, A, B, C and client A, B
3) Write to file: https://www.w3schools.com/cpp/cpp_files.asp
Found in void writeTXLIST(string list, int size) from serverM
4) Read in file line by line: https://www.cplusplus.com/doc/tutorial/files/
Found in int checkWallet(string usrnme) from server A, B, C
5) stoi function: https://stackoverflow.com/questions/19311641/c-string-to-int-without-using-atoi-or-stoi; https://www.geeksforgeeks.org/converting-strings-numbers-cc/
Found in int stoint(string s) from server M, A, B, C, client A, B
6) to_string function: https://stackoverflow.com/questions/4668760/converting-an-int-to-stdstring
Found in string to_string(int x, int y) from server M, A, B, C, client A, B