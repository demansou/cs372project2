"""
Author: Daniel Mansour
Creation Date: 08-01-2016
File Name: ftclient.py
Description:
"""

import sys      # to access command line args
import socket	# to access socket functionality
import re       # to use regex
import os       # to use path functionality

TEST = True

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
    ft_command = sys.argv[3]
    if sys.argv[3] == "-g":
        ft_filename = sys.argv[4]
        ft_dataport = sys.argv[5]
        return ft_command, ft_filename, ft_dataport
    return ft_command, None, None


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


def sendCommand(sock, ft_command, ft_filename, ft_dataport):
    """
    Sends command line commands to ftserver
    """
    try:
        sock.send(ft_command)
    except:
        print >> sys.stderr, "[ftclient] ERROR! Did not send command to server"
        sys.exit(1)
    data = sock.recv(64)
    if TEST:
        print >> sys.stderr, "[DEBUG] data received: %s" % data
    if ft_command == "-g" and "-g" in data:
        try:
            sock.send(ft_filename)
        except:
            print >> sys.stderr, "[ftclient] ERROR! Did not send filename to server"
            sys.exit(1)
        data = sock.recv(64)
        if TEST:
            print >> sys.stderr, "[DEBUG] data received: %s" % data
        try:
            sock.send(ft_dataport)
        except:
            print >> sys.stderr, "[ftclient] ERROR! Did not send dataport to server"
            sys.exit(1)
        data = sock.recv(64)
        if TEST:
            print >> sys.stderr, "[DEBUG] data received: %s" % data
    return


def getResponse(sock, ft_command, ft_filename, ft_dataport):
    """
    gets response based on command sent to ftserver
    '-l' returns a list of txt files
    '-g' returns a file object over socket if file exists
    """
    if ft_command == "-l":
        filelist = []
        while True:
            filelist.append(sock.recv(64))
            if TEST:
                print >> sys.stderr, "[DEBUG] data received: %s" % filelist[len(filelist) - 1]
            try:
                sock.send(filelist[len(filelist) - 1])
            except:
                print >> sys.stderr, "[ftclient] ERROR! did not respond to text file listing from server"
            if "endlist" in filelist[len(filelist) - 1]:
                break
        print("List of text files on server:")
        for item in filelist:
            if "endlist" not in item:
                print("%s" % item)
    elif ft_command == "-g":
        sock.send(socket.gethostbyname(socket.gethostname()))
        myip = sock.recv(16)
        if TEST:
            print >> sys.stderr, "[DEBUG] ip received: %s" % myip
        """
        sock2 = createServer(socket.gethostbyname(socket.gethostname()), ft_dataport)
        sock2.listen(1)
        print >> sys.stderr, "[ftclient] listening on: %s:%s" % (socket.gethostbyname(socket.gethostname()), ft_dataport)
        while True:
            try:
                client_connection, client_address = sock2.accept()
            except:
                print >> sys.stderr, "[ftclient] ERROR! could not receive incoming connection"
            print >> sys.stderr, "[ftclient] connected to %s" % client_address
        """
        filedata = ""
        part = None
        while part != "":
            part = sock.recv(1024)
            filedata += part
        if TEST:
            print >> sys.stderr, "%s" % filedata
        textfile = open(ft_filename, "w")
        textfile.write("%s\n" % filedata)
        textfile.close()
    return


def runServer():
    """

    """
    server_addr, server_port = getServerAddr()
    ft_command, ft_filename, ft_dataport = getFTCommands()
    sock = createSocket(server_addr, server_port)
    sendCommand(sock, ft_command, ft_filename, ft_dataport)
    getResponse(sock, ft_command, ft_filename, ft_dataport)
    return


def main():
    """
    Main function calls runServer()
    """
    runServer()
    sys.exit(0)

if __name__ == "__main__":
    main()
