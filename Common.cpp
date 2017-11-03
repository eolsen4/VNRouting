#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Common.hpp"

using namespace std;

/* function to get the data port number for a specific node in the given file and node number */
int getDataPort(string filename, int node){
    string line;
    ifstream file(filename.c_str());

    int found_node;
    int data_port = 0;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            if(!(iss >> unused_data >> unused_data >> data_port)){ break; }
        }
    }
    return data_port;
}


/* function to get the control port number for a specific node in the given file and node number */
int getContPort(string filename, int node){
    string line;
    using namespace std;ifstream file(filename.c_str());

    int found_node;
    int cont_port = 0;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            if(!(iss >> unused_data >> cont_port)){ break; }
        }
    }
    return cont_port;
}


/* function to get the hostname for a specific node in the given file and node number */
string getHostname(string filename, int node){
    string line;
    ifstream file(filename);

    int found_node;
    string hostname;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            if(!(iss >> hostname)){ break; }
        }
    }
    return hostname;
}


/* function to get a specific node's neighbors with their data port numbers in the given file */
vector<pair<int, int> > getAdjacentDataPorts(string filename, int node){
    string line;
    ifstream file(filename);

    vector<pair<int, int> > adj_data_ports;
    vector<int> adj_nodes;
    int found_node;
    int neighbor_port;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            // skip hostname, control port, and data port
            if(!(iss >> unused_data >> unused_data >> unused_data)){ break; }

            while(iss >> neighbor_port)
            {
                adj_nodes.push_back(neighbor_port);
            }
            break;
        }
    }

    int itr = 0;
    int data_port;

    for(int i = 0; i < adj_nodes.size(); ++i)
    {
        data_port = getDataPort(filename, adj_nodes[i]);
        adj_data_ports.push_back(pair<int, int>(adj_nodes[i],data_port));
    }

    return adj_data_ports;
}


/* function to get a specific node's neighbors with their control port numbers in the given file */
vector<pair<int, int> > getAdjacentContPorts(string filename, int node){
    string line;
    ifstream file(filename);

    vector<pair<int, int> > adj_cont_ports;
    vector<int> adj_nodes;
    int found_node;
    int neighbor_port;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            // skip hostname, control port, and data port
            if(!(iss >> unused_data >> unused_data >> unused_data)){ break; }

            while(iss >> neighbor_port)
            {
                adj_nodes.push_back(neighbor_port);
            }
            break;
        }
    }

    int itr = 0;
    int cont_port;

    for(int i = 0; i < adj_nodes.size(); ++i)
    {
        cont_port = getContPort(filename, adj_nodes[i]);
        adj_cont_ports.push_back(pair<int, int>(adj_nodes[i],cont_port));
    }

    return adj_cont_ports;
}


/* function to get a specific node's neighbors with their hostnames in the given file */
vector<pair<int, string> > getAdjacentHostnames(string filename, int node){
    string line;
    ifstream file(filename);

    vector<pair<int, string> > adj_hostnames;
    vector<int> adj_nodes;
    int found_node;
    int neighbor_port;
    string unused_data;
    
    while(getline(file, line))
    {
        istringstream iss(line);
        if(!(iss >> found_node)){ break; }
        if(node == found_node)
        {
            // skip hostname, control port, and data port
            if(!(iss >> unused_data >> unused_data >> unused_data)){ break; }

            while(iss >> neighbor_port)
            {
                adj_nodes.push_back(neighbor_port);
            }
            break;
        }
    }

    int itr = 0;
    string hostname;

    for(int i = 0; i < adj_nodes.size(); ++i)
    {
        hostname = getHostname(filename, adj_nodes[i]);
        adj_hostnames.push_back(pair<int, string>(adj_nodes[i],hostname));
    }

    return adj_hostnames;
}
