from influxdb import InfluxDBClient
import sys


if (len(sys.argv) < 2):
  print("usage: python influxDB.py <option>\n possible options: create, dump, listDB");
  exit()


if(sys.argv[1] == "create"):
  if(len(sys.argv) != 5):
    print("usage python influxDB.py create <host> <port> <name>")
    exit()
  print("creating database")
  client = InfluxDBClient(host=sys.argv[2], port=int(sys.argv[3]))
  client.create_database(sys.argv[4])

elif(sys.argv[1] == "listDB"):
  if(len(sys.argv) != 4):
    print("usage python influxDB.py create <host> <port>")
    exit()
  print("listing all DBs:")
  client = InfluxDBClient(host=sys.argv[2], port=int(sys.argv[3]))
  print(client.get_list_database())
  
  
