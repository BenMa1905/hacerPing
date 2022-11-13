#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

class PingState
{
public:
    string ip;
    int enviados;
    int recibidos;
    int perdidos;
    string estado;
};

void createPing(PingState *dataPing, string cant)
{
    string comm = "ping " + dataPing->ip + " -c " + cant + " -q";

    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(comm.c_str(), "r"), pclose);

    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    if (result == "")
    {
        throw std::runtime_error("El ping fallo");
    }
    
    char *aux;
    char *paqRecibidos;

    char *output = new char[result.length() + 1];//se pasa de string a char* para poder usar strtok

    strcpy(output, result.c_str());
    aux = strtok(output, " ");
    do
    {
        aux = strtok(NULL, " ");

        if (strcmp(aux, "received,") == 0)
        {
            dataPing->recibidos = atoi(paqRecibidos);//guarda los paquetes recibidos
        }else
        {
            paqRecibidos = aux;
        }

    }while (strcmp(aux, "received,") != 0);

    dataPing-> enviados = atoi(cant.c_str());

    // calcula y guarda la cantidad de paquetes perdidos
    dataPing->perdidos = dataPing-> enviados - dataPing->recibidos;

    if (dataPing->recibidos >= 1)
    {
        dataPing->estado = "UP";
    }
    else
    {
        dataPing->estado = "DOWN";
    }

    if (dataPing->ip.length() >= 15)
    {
        cout << dataPing->ip << " \t" << dataPing -> enviados << " \t" << dataPing->recibidos << " \t" << dataPing->perdidos << " \t" << dataPing->estado << endl;
    }
    else
    {
        cout << dataPing->ip << " \t\t" << dataPing-> enviados << " \t" << dataPing->recibidos << " \t" << dataPing->perdidos << " \t" << dataPing->estado << endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "No se ingresaron los argumentos suficientes (2)" << endl;
        return 1;
    }
    string nombreArchivo = argv[1];
    if (nombreArchivo.find(".txt") == std::string::npos)
    {
        cout << "No se ingreso un archivo .txt" << endl;
        return 1;
    }

    try
    {
        int numero = stoi(argv[2]);
    }catch (std::exception const & e)
    {
        cout << "La cantidad de paquetes debe ser un numero entero" << endl;
        return 1;
    }

    string cantPackages = argv[2];
    
    std::ifstream ipFile;//Se declara el tipo de archivo
    ipFile.open(argv[1]);//Se abre el archivo

    std::string ip;
    int count = 0;//Variable para contar las lineas del archivo de texto

    if (ipFile.is_open())
    {
        while (ipFile.good())
        {
            ipFile >> ip;
            count++;
        }
    }

    ipFile.close();
    ipFile.open(argv[1]);

    PingState pings[count];
    count = 0;
    if (ipFile.is_open())
    {
        while (ipFile.good())
        {
            ipFile >> ip;
            pings[count].ip = ip;
            count++;
        }
    }

    thread threads[count];//Se crean los hilos

    cout << "IP \t\t   Env. \tRec. \t Perd. \t Estado " << endl;
    cout << "-------------------------------------------------------"<< endl;

    //Se crean los hilos
    for (int i = 0; i < count; i++)
    {
        threads[i] = std::thread(&createPing, &pings[i], cantPackages);
    }

    //El main espera a que se terminen de ejecutar los hilos
    for (int i = 0; i < count; i++)
    {
        threads[i].join();
    }

    return 0;
}