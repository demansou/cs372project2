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
        print >> sys.stderr, "[ftclient] ERROR! Correct format 'ftclient [server address] [server port] <[command] [filename] [data port]>'"
        sys.exit(2)
    server_addr = sys.argv[1]
    server_port = int(sys.argv[2])
    return server_addr, server_port


def getFTCommands()
    """
    Returns command line file transfer arguments command, filename, and data port
    """
    if len(sys.argv) < 4:
        print >> sys.stderr, "[ftclient] ERROR! No command detected"
        sys.exit(1)
    if len(sys.argv) >= 4:
        ft_command = sys.argv[3]
    if len(sys.argv) >= 4:
        ft_filename = sys.argv[4]
    if len(sys.argv) >= 5:
        ft_dataport = sys.argv[5]
    return ft_command, ft_filename, ft_dataport


def runServer():
    """
    """
    return


def main():
    """
    Main function calls runServer()
    """
    runServer()
    sys.exit(0)

if __name__ == "__main__":
    main()
