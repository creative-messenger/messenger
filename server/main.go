package main

import (
	"encoding/json"
	"fmt"
	"net"
	"os"
)

type ReqResp struct {
	Method string
	Object string
	Param string
	Msg Message
}

type Message struct {
	UserFrom string
	UserTo string
	Text string
}

type Config struct {
	Users []User
}
type User struct {
	Login string
	Addr  net.Addr
}
var users []User
func getUsers() *Config{
	file, _ := os.Open("users.json")
	config := new(Config)
	json.NewDecoder(file).Decode(&config)
	return config
}

func isContactExist(contact string) bool {
	//config := new(Config)
	//config = getUsers()

	for _, us := range users{
		if us.Login == contact {
			return true
		}
	}
	return false
}

func checkContact(contact string, resp ReqResp) []byte{

	if isContactExist(contact) {
		resp.Param = "yes"
	} else {
		resp.Param = "no"
	}
	b, _ := json.Marshal(resp)
	return b
}

func addUser(user User, resp ReqResp) []byte {

	for _, val := range users {
		fmt.Println(val)
		if val.Login == user.Login{
			resp.Param = "no"
			break
		} else {
			resp.Param = "yes"
		}
	}

	if len(users) == 0 {
		resp.Param = "yes"
	}

	if resp.Param == "yes"{
		users = append(users, user)
	}
	b, _ := json.Marshal(resp)
	return b
}

func sendMessage(msg Message, resp ReqResp) []byte {
	user := msg.UserTo
	for _, val := range users {
		fmt.Println(val)
		if val.Login == user{
			resp.Param = "ok"
		}
	}
	b, _ := json.Marshal(resp)
	return b
}

func initReq(req ReqResp, conn net.Conn) []byte{
	var resp ReqResp
	resp.Object = req.Object
	resp.Method = req.Method
	resp.Msg = req.Msg
	if req.Method == "GET" && req.Object == "contact" {
		b := checkContact(req.Param, resp)
		return b
	}

	if req.Method == "POST" && req.Object == "contact" {
		var newUser User
		newUser.Login = req.Param
		newUser.Addr = conn.RemoteAddr()
		b := addUser(newUser, resp)
		return b
	}
	if req.Method == "POST" && req.Object == "message" {
		b := sendMessage(req.Msg, resp)
		return b
	}

	return []byte("error")
}

func handleConnection(conn net.Conn) {
	var req ReqResp
	for {
		buf := make([]byte, 1024)
		n, _ := conn.Read(buf)
		buf = buf[0:n]
		if len(buf) > 0 {
			fmt.Println("Пришел запрос: ", string(buf))
			err := json.Unmarshal(buf, &req)
			if err != nil {
				fmt.Println("err: ", err)
			} else {
				conn.Write(initReq(req, conn))
			}
		} else {
			if len(users) > 0 {
				var x int
				for n, val := range users {
					if val.Addr == conn.RemoteAddr() {
						x = n
					}
				}
				users = append(users[:x], users[x+1:]...)
			}
			fmt.Println("Пользователь ", conn.RemoteAddr(), " отключился", )
			break
		}



	}
}
func main() {

	// Устанавливаем прослушивание порта
	ln, _ := net.Listen("tcp", ":8080")

	fmt.Println("Listen...")

	// Запускаем цикл
	for {
		conn, _ := ln.Accept()      // Открываем порт

		go handleConnection(conn)

	}
}