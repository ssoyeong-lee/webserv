#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <stdint.h>
#include <cstring>
#include <cstdio>

#include <ConfigUtils.hpp>

using namespace std;

class Module
{
protected:
    const string            name;
    const enum ModuleType   type;
    vector<Module*>         subMods;
    
public:
    Module ( Derivative const & deriv, enum ModuleType type )
     : name(deriv.name), type(type) {}

    const string & getName( void ) const { return name; }
    const enum ModuleType & getType( void ) const { return type; }
    const vector<Module*> & getSubMods( void ) const { return subMods; }

    void addModule( Module * m ) { subMods.push_back(m); }
};

class MainModule : public Module
{
public:
	MainModule( const Derivative & deriv ) : Module(deriv, NO_TYPE) {}
};

class ServerModule : public Module
{
private:
    uint32_t  ip;
    int     port;
    vector<string> serverNames;

    void setListen( const Derivative & deriv )
    {
        // check syntax
        size_t delimIdx = deriv.arg[1].find(':');

        ip = ipToInt(deriv.arg[1].substr(0, delimIdx));
        port = atoi(deriv.arg[1].substr(delimIdx + 1).c_str());
    }
    void setServerName( const Derivative & deriv )
    {
        // check syntax
        for (size_t i = 1; i < deriv.arg.size(); i++)
            this->serverNames.push_back(deriv.arg[i]);
    }
    uint32_t ipToInt(const std::string& ip) {
    char ipCStr[16];
    std::strncpy(ipCStr, ip.c_str(), sizeof(ipCStr));
    ipCStr[sizeof(ipCStr) - 1] = '\0'; // 문자열 끝에 null 문자 추가

    int part1, part2, part3, part4;
    if (std::sscanf(ipCStr, "%d.%d.%d.%d", &part1, &part2, &part3, &part4) != 4) {
        // 잘못된 형식의 IP 주소를 입력받은 경우 예외 처리
        throw std::invalid_argument("Invalid IP address format.");
    }

    if (part1 < 0 || part1 > 255 || part2 < 0 || part2 > 255 ||
        part3 < 0 || part3 > 255 || part4 < 0 || part4 > 255) {
        // 유효하지 않은 IP 주소 부분이 입력된 경우 예외 처리
        throw std::invalid_argument("Invalid IP address value.");
    }

    uint32_t result = (static_cast<uint32_t>(part1) << 24) |
                      (static_cast<uint32_t>(part2) << 16) |
                      (static_cast<uint32_t>(part3) << 8) |
                      static_cast<uint32_t>(part4);

    return result;
}
public:
	ServerModule( const Derivative & deriv, vector<Derivative> subDerivs )
     : Module(deriv, NO_TYPE)
    {
        setListen(subDerivs[0]);
        setServerName(subDerivs[1]);
    }
    
    const uint32_t & getIp( void ) const { return ip; }
    const vector<string> & getServerNames( void ) const { return serverNames; }
    int getPort( void ) const { return port; }
};

class LocationModule : public Module
{
private:
    string uri;
public:
    LocationModule( const Derivative & deriv ) : Module(deriv, NO_TYPE)
    {
        // check syntax
        uri = deriv.arg[1];
        if (uri.back() == '/')
            uri.pop_back();
    }

    const string & getUri( void ) const { return uri; }
};

class RootModule : public Module
{
private:
    string root;
public:
    RootModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        // check syntax
        root = deriv.arg[1];
    }

    const string & getRoot( void ) const { return root; }
};

class TypesModule : public Module
{
private:
    map<string, string> typesMap;
public:
    TypesModule( const Derivative & deriv, vector<Derivative> subDerivs )
     : Module(deriv, LOC_MOD)
    {
        for (size_t i = 0; i < subDerivs.size(); i++) {
            // check syntax
            string type = subDerivs[i].arg[0];

            for (size_t j = 1; j < subDerivs[i].arg.size(); j++) {
                string extension = subDerivs[i].arg[j];

                typesMap[extension] = type;
            }
        }
    }

    const map<string, string> & getTypesMap( void ) const { return typesMap; }
};

class IndexModule : public Module
{
private:
    vector<string> indexes;
public:
    IndexModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        // check syntax
        for (size_t i = 1; i < deriv.arg.size(); i++)
            indexes.push_back(deriv.arg[i]);
    }

    const vector<string> & getIndexes( void ) const { return indexes; }
};

class ErrorPageModule : public Module
{
private:
    vector<int> errCodes;
    string uri;
public:
    ErrorPageModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        // check syntax
        for (size_t i = 1; i < deriv.arg.size() - 1; i++)
            errCodes.push_back(atoi(deriv.arg[i].c_str()));
        
        uri = deriv.arg.back();
    }

    bool isErrCode( int code ) const 
    {
        if (find(errCodes.begin(), errCodes.end(), code) != errCodes.end())
            return true;

        return false;
    }
    const string & getUri( void ) const { return uri; }
};

class CgiModule : public Module
{
private:
    string cgiCmd;
public:
    CgiModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        // check syntax
        cgiCmd = deriv.arg[1];
    }

    const string & getCgiCmd( void ) const { return cgiCmd; }
};

class CgiParamsModule : public Module
{
private:
    vector<pair<string, string> > params;
public:
    CgiParamsModule( const Derivative & deriv, const vector<Derivative> & subDerivs ) : Module(deriv, LOC_MOD)
    {
        for (int i = 0; i < subDerivs.size(); i++)
        {
            // check syntax
            params.push_back(make_pair(subDerivs[i].arg[0], subDerivs[i].arg[1]));
        }
    }

    const vector<pair<string, string> > & getParams( void ) const { return params; }
};

class AutoIndexModule : public Module
{
private:
    bool isAutoIndex;
public:
    AutoIndexModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        // check syntax
        if (deriv.arg[1] == "on")
            isAutoIndex = true;
        else
            isAutoIndex = false;
    }

    bool getAutoIndex( void ) const { return isAutoIndex; }
};

class ClientMaxBodySizeModule : public Module
{
private:
    int maxSize;
public:
    ClientMaxBodySizeModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        maxSize = atoi(deriv.arg[1].c_str());
    }

    int getClientMaxBodySize( void ) const { return maxSize; }
};

class AcceptMethodModule : public Module
{
private:
    vector<string> methods;
public:
    AcceptMethodModule( const Derivative & deriv ) : Module(deriv, LOC_MOD)
    {
        for (int i = 1; i < deriv.arg.size(); i++)
        {
            // check syntax
            methods.push_back(deriv.arg[i]);
        }
    }

    const vector<string> & getAcceptMethods( void ) const { return methods; }
};
