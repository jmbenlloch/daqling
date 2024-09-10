package main

import (
	"encoding/binary"
	"flag"
	"fmt"
	"math/rand/v2"
	"net"
	"os"
	"time"
)

func ldcConnection(ip string, port int) *net.UDPConn {
	// Dial to the address with UDP
	address := fmt.Sprintf("%s:%d", ip, port)
	fmt.Println(address)
	udpAddr, err := net.ResolveUDPAddr("udp", address)
	conn, err := net.DialUDP("udp", nil, udpAddr)

	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	return conn
}

func sendEvent(conn *net.UDPConn, nPackets int, eventID int) {
	dataLog := make([][]byte, nPackets)
	for packetIdx := 0; packetIdx < nPackets; packetIdx++ {
		// Test data
		//size := 9000
		size := 8950
		subevent := make([]byte, size)
		value := rand.IntN(254) + 1
		if value == 0xfa {
			value++
		}
		for i := 0; i < size; i++ {
			subevent[i] = byte(value)
		}

		binary.LittleEndian.PutUint32(subevent[0:4], uint32(packetIdx))
		binary.LittleEndian.PutUint32(subevent[4:8], uint32(eventID))
		//fmt.Printf("Evt %d subevent %d:\t %x \n", eventID, packetIdx, subevent[0:8])
		dataLog[packetIdx] = subevent[0:12]

		// Send a message to the server
		_, err := conn.Write(subevent)
		if err != nil {
			fmt.Println(err)
			fmt.Printf("error data: %d\n", packetIdx)
			for i := 0; i < nPackets; i++ {
				fmt.Printf("error data[%d]: %x\n", i, dataLog[i])
			}
			os.Exit(1)
		}
	}

	fmt.Printf("Evt %d\n", eventID)
	for i := 0; i < nPackets; i++ {
		fmt.Printf("error data[%d]: %x\n", i, dataLog[i])
	}

	size := 4
	subevent := make([]byte, size)
	binary.LittleEndian.PutUint32(subevent[size-4:size], 0xfafafafa)
	//fmt.Printf("subevent:\t %x \n", subevent)
	// Send a message to the server
	_, err := conn.Write(subevent)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func main() {
	host := flag.String("host", "localhost", "Server IP")
	port := flag.Int("port", 6123, "Server port")
	flag.Parse()

	conn := ldcConnection(*host, *port)
	evtCounter := 1
	for {
		fmt.Println("Evt: ", evtCounter)
		//nPackets := 10
		//nPackets := 100
		nPackets := 50
		//nPackets := rand.IntN(10) + 1
		fmt.Println(nPackets)
		sendEvent(conn, nPackets, evtCounter)
		//time.Sleep(1 * time.Millisecond)
		//time.Sleep(100 * time.Microsecond)
		time.Sleep(6 * time.Millisecond)
		evtCounter++
	}
}
