"""
Author: Daniel Mansour
Creation Date: 08-01-2016
File Name: ftclient.py
Description: ftclient takes a command line argument in the format
             'python ftclient.py [server ip] [server port] [command] [filename <only if using -g>] [data port]'
             server ip is ip of server to connect to
             server port is access port to communicate with server
             command is '-l' to list text files or '-g' to get a text file if it exists
             data port is port to return either list of text files or text file contents on specific client port
"""

import sys      # to access command line args
import socket	# to access socket functionality
import re       # to use regex
import os       # to use path functionality

# TEST = True
TEST = False

def getServerAddr():
    """
    Returns argument for server ip address and server listening port
    """
    if len(sys.argv) < 3:
        print >> sys.stderr, "[ftclient] ERROR! Correct format 'python ftclient.py [server address] [server port] <[command] [filename] [data port]>'"
        sys.exit(2)
    server_addr = sys.argv[1]
    server_port = int(sys.argv[2])
    return server_addr, server_port


def getFTCommands():
    """
    Returns command line file transfer arguments command, filename, and data port
    """
    if len(sys.argv) < 4:
        print >> sys.stderr, "[ftclient] ERROR! No command detected"
        sys.exit(2)
    elif sys.argv[3] == "-g" and len(sys.argv) < 6:
        print >> sys.stderr, "[ftclient] ERROR! When using '-g', must include filename and data port"
        print >> sys.stderr, "[ftclient] Correct format 'python ftclient.py [server address] [server port] [-g] [filename] [data port]'"
    elif sys.argv[3] == "-l" and len(sys.argv) < 5:
        print >> sys.stderr, "[ftclient] ERROR! when using '-l', must include data port"
        print >> sys.stderr, "[ftclient] correct format 'python ftclient.py [server address] [server port] [-l] [data port]'"
    ft_command = sys.argv[3]
    if sys.argv[3] == "-g":
        ft_filename = sys.argv[4]
        ft_dataport = sys.argv[5]
        return ft_command, ft_filename, ft_dataport
    elif sys.argv[3] == "-l":
        ft_dataport = sys.argv[4]
        return ft_command, None, ft_dataport


def createSocket(server_addr, server_port):
    """
    Creates socket and connects to server ip address and port
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((server_addr, int(server_port)))
    except:
        print >> sys.stderr, "[ftclient] ERROR! Did not connect to %s:%s" % (server_addr, server_port)
        sys.exit(1)
    return sock


def createServer(server_addr, server_port):
    """
    Creates socket and listens on port
    """
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock2.bind((server_addr, int(server_port)))
    except:
        print >> sys.stderr, "[ftclient] ERROR! did not bind to %s:%s" % (server_addr, server_port)
        sys.exit(1)
    return sock2


def sendCommand(sock, ft_command, ft_dataport):
    """
    Sends command line command and data port to ftserver and establishes data connection
    """
    try:
        sock.send(ft_command)
    except:
        print >> sys.stderr, "[ftclient] ERROR! Did not send command to server"
        sys.exit(1)
    data = sock.recv(64)
    if TEST:
        print >> sys.stderr, "[DEBUG] data received: %s" % data
    if ft_command == "-g" or ft_command == "-l":
        # send client ip to server
        try:
            sock.send(socket.gethostbyname(socket.gethostname()))
        except:
            print >> sys.stderr, "[ftclient] ERROR! did not send client ip to server"
            sys.exit(1)
        clientip = sock.recv(16)
        if TEST:
            print >> sys.stderr, "[DEBUG] client ip: %s" % clientip
        # send client data port (from command line input) to server
        try:
            sock.send(ft_dataport)
        except:
            print >> sys.stderr, "[ftclient] ERROR! did not send client dataport to server"
            sys.exit(1)
        dataport = sock.recv(6)
        if TEST:
            print >> sys.stderr, "[DEBUG] client dataport %s" % dataport
        sock2 = createServer(socket.gethostbyname(socket.gethostname()), ft_dataport)
        sock2.listen(1)
        sock.send("connect")
        client_connection, client_address = sock2.accept()
        if TEST:
            print >> sys.stderr, "connected to %s %s" % (client_address[0], client_address[1])
    return client_connection


def getResponse(client_connection, ft_command, ft_filename):
    """
    gets response based on command sent to ftserver
    '-l' returns a list of txt files
    '-g' returns a file object over socket if file exists
    """
    if ft_command == "-l":
        filelist = []
        while True:
            filelist.append(client_connection.recv(64))
            if TEST:
                print >> sys.stderr, "[DEBUG] data received: %s" % filelist[len(filelist) - 1]
            try:
                client_connection.send(filelist[len(filelist) - 1])
            except:
                print >> sys.stderr, "[ftclient] ERROR! did not respond to text file listing from server"
            if "endlist" in filelist[len(filelist) - 1]:
                break
        print("List of text files on server:")
        for item in filelist:
            if "endlist" not in item:
                print("%s" % item)
    elif ft_command == "-g":
        try:
            client_connection.send(ft_filename)
        except:
            print >> sys.stderr, "[ftclient] ERROR! did not send filename to server"
        if TEST:
            fprintf >> sys.stderr, "[DEBUG] file to send: %s" % data
        part = None
        filedata = ""
        while part != "":
            part = client_connection.recv(64)
            filedata += part
            if TEST:
                print >> sys.stderr, "reached end of while loop"
        if TEST:
            print >> sys.stderr, "%s" % filedata
        if "Server says FILE NOT FOUND" not in filedata:
            print("Receiving file from server...")
            textfile = open(ft_filename, "w")
            textfile.write("%s" % filedata)
            textfile.close()
            print("File transfer complete...")
        else:
            print("%s" % filedata)
    return


def runServer():
    """
    gathers server ip and port from command line
    gathers '-l' or '-g' and data port
    if '-g', gathers filename
    sends command '-l' or '-g' to server along with data port
    if '-g', sends filename to server
    receives response from server based on sent data
    """
    server_addr, server_port = getServerAddr()
    ft_command, ft_filename, ft_dataport = getFTCommands()
    sock = createSocket(server_addr, server_port)
    client_connection = sendCommand(sock, ft_command, ft_dataport)
    getResponse(client_connection, ft_command, ft_filename)
    return


def main():
    """
    Main function calls runServer()
    """
    runServer()
    sys.exit(0)

if __name__ == "__main__":
    main()
