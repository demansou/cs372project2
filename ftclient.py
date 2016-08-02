"""
Author: Daniel Mansour
Creation Date: 08-01-2016
File Name: ftclient.py
Description:
"""

import sys      # to access command line args
import socket	# to access socket functionality
import re       # to use regex


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


def sendCommand(sock, ft_command, ft_filename, ft_dataport):
    """
    Sends command line commands to ftserver
    """
    if ft_command == "-l" or ft_command == "-g":
        try:
            sock.send(ft_command)
        except:
            print >> sys.stderr, "[ftclient] ERROR! Did not send command to server"
            sys.exit(1)
    if ft_command == "-g":
        try:
            sock.send(ft_filename)
        except:
            print >> sys.stderr, "[ftclient] ERROR! Did not send filename to server"
            sys.exit(1)
        try:
            sock.send(ft_dataport)
        except:
            print >> sys.stderr, "[ftclient] ERROR! Did not send dataport to server"
            sys.exit(1)
    return


def runServer():
    """

    """
    server_addr, server_port = getServerAddr()
    ft_command, ft_filename, ft_dataport = getFTCommands()
    sock = createSocket(server_addr, server_port)
    sendCommand(sock, ft_command, ft_filename, ft_dataport)
    #getResponse()
    return


def main():
    """
    Main function calls runServer()
    """
    runServer()
    sys.exit(0)

if __name__ == "__main__":
    main()
