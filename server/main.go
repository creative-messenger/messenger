package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"strings"
)

type ReqResp struct {
	Method string
	Object string
	Param  string
	Msg    Message
	List   []string
}

type Message struct {
	UserFrom string
	UserTo   string
	Text     string
}

type Config struct {
	Users []User
}
type User struct {
	Login    string    `json:"Login"`
	Password string    `json:"Password"`
	Contacts []Contact `json:"Contacts"`
}

type Contact struct {
	Name     string   `json:"Name"`
	Messages []string `json:"Messages"`
}

var users []User

const filename string = "users.json"

func getUsers() *Config {
	file, _ := os.Open(filename)
	config := new(Config)
	json.NewDecoder(file).Decode(&config)
	return config
}

func isContactExist(contact string) bool {
	config := new(Config)
	config = getUsers()

	for _, us := range config.Users {
		if us.Login == contact {
			return true
		}
	}
	return false
}

func getContacts(login string) []string {
	config := new(Config)
	config = getUsers()
	var contacts []string
	for _, us := range config.Users {
		if us.Login == login{
			for _, c := range us.Contacts {
				contacts = append(contacts, c.Name)
			}
			return contacts
		}
	}
	return contacts
}

func isUserExist(login, password string) bool {
	config := new(Config)
	config = getUsers()
	for _, us := range config.Users {
		if us.Login == login && us.Password == password {
			return true
		}
	}
	return false
}

func appendContact(user, contact string) {
	rawDataIn, _ := ioutil.ReadFile(filename)

	var config Config
	json.Unmarshal(rawDataIn, &config)

	newContact := Contact{
		Name:     contact,
		Messages: nil,
	}
	for i, us := range config.Users {
		if us.Login == user {
			us.Contacts = append(us.Contacts, newContact)
		}
		config.Users[i] = us
	}

	rawDataOut, _ := json.MarshalIndent(&config, "", "  ")

	ioutil.WriteFile(filename, rawDataOut, 0644)
}

func addContact(user string, resp ReqResp) []byte {
	data := strings.Split(user,"/")
	user = data[0]
	contact := data[1]

	if isContactExist(contact) {
		resp.Param = "yes"
		// добавление контакта в users.json
		appendContact(user, contact)
	} else {
		resp.Param = "no"
	}
	b, _ := json.Marshal(resp)
	return b
}

func delContact(user string, resp ReqResp) []byte {
	data := strings.Split(user,"/")
	user = data[0]
	contact := data[1]

	// удаление контакта из users.json
	rawDataIn, _ := ioutil.ReadFile(filename)
	var config Config
	json.Unmarshal(rawDataIn, &config)

	for i, us := range config.Users {
		if us.Login == user {
			for i, currContact := range us.Contacts {
				if currContact.Name == contact {
					us.Contacts = append(us.Contacts[:i], us.Contacts[i+1:]...)
				}
			}
		}
		config.Users[i] = us
	}
	rawDataOut, _ := json.MarshalIndent(&config, "", "  ")
	ioutil.WriteFile(filename, rawDataOut, 0644)


	resp.Param = "ok"
	b, _ := json.Marshal(resp)
	return b
}

func checkUser(user string, resp ReqResp) []byte {
	data := strings.Split(user, "/")

	if isUserExist(data[0], data[1]) {
		resp.Param = "yes"
		resp.List = getContacts(data[0])
	} else {
		resp.Param = "no"
	}

	b, _ := json.Marshal(resp)
	return b
}

func appendUser(login, password string) {

	rawDataIn, _ := ioutil.ReadFile(filename)

	var config Config
	json.Unmarshal(rawDataIn, &config)

	newUser := User{
		Login:    login,
		Password: password,
		Contacts: nil,
	}

	config.Users = append(config.Users, newUser)

	rawDataOut, _ := json.MarshalIndent(&config, "", "  ")

	ioutil.WriteFile(filename, rawDataOut, 0644)
}

func addUser(user string, resp ReqResp) []byte {
	data := strings.Split(user, "/")
	login := data[0]
	password := data[1]
	if isContactExist(login) {
		resp.Param = "no"
	} else {
		resp.Param = "yes"
	}

	if resp.Param == "yes" {
		appendUser(login, password)
	}
	b, _ := json.Marshal(resp)
	return b
}

func addMessage(user1, user2, text string) {
	rawDataIn, _ := ioutil.ReadFile(filename)
	find := false

	newContact := Contact{
		Name:     user2,
		Messages: nil,
	}
	newContact.Messages = append(newContact.Messages, text)

	var config Config
	json.Unmarshal(rawDataIn, &config)
	for i, us := range config.Users {
		if us.Login == user1 {
			for i, currContact := range us.Contacts {
				if currContact.Name == user2 {
					us.Contacts[i].Messages = append(us.Contacts[i].Messages, text)
					find = true
				}
			}
			if !find {
				us.Contacts = append(us.Contacts, newContact)
			}
		}
		config.Users[i] = us
	}

	rawDataOut, _ := json.MarshalIndent(&config, "", "  ")

	ioutil.WriteFile(filename, rawDataOut, 0644)
}

func sendMessage(msg Message, resp ReqResp) []byte {
	userFrom := msg.UserFrom
	userTo := msg.UserTo
	text := msg.Text

	addMessage(userFrom, userTo, text)
	addMessage(userTo, userFrom, text)

	b, _ := json.Marshal(resp)
	return b
}

func getMessages(login, contact string) []string {
	config := new(Config)
	config = getUsers()
	var messages []string
	for _, us := range config.Users {
		if us.Login == login {
			for _, c := range us.Contacts {
				if c.Name == contact {
					messages = c.Messages
				}
			}
			return messages
		}
	}
	return messages
}

func sendMessages(user string, resp ReqResp) []byte {
	data := strings.Split(user, "/")
	messages := getMessages(data[0], data[1])
	if len(messages) > 0 {
		resp.Param = "yes"
	} else {
		resp.Param = "no"
	}
	resp.List = messages
	b, _ := json.Marshal(resp)
	return b

}

func initReq(req ReqResp, conn net.Conn) []byte {
	var resp ReqResp
	resp.Object = req.Object
	resp.Method = req.Method
	resp.Msg = req.Msg
	if req.Method == "POST" && req.Object == "contact" {
		b := addContact(req.Param, resp)
		return b
	}
	if req.Method == "POST" && req.Object == "message" {
		b := sendMessage(req.Msg, resp)
		return b
	}
	if req.Method == "GET" && req.Object == "user" {
		b := checkUser(req.Param, resp)
		return b
	}
	if req.Method == "GET" && req.Object == "messages" {
		b := sendMessages(req.Param, resp)
		return b
	}
	if req.Method == "POST" && req.Object == "user" {
		b := addUser(req.Param, resp)
		return b
	}
	if req.Method == "DELETE" && req.Object == "contact" {
		b := delContact(req.Param, resp)
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
			/*
				if len(users) > 0 {

					var x int
					for n, val := range users {
						if val.Addr == conn.RemoteAddr() {
							x = n
						}
					}
					users = append(users[:x], users[x+1:]...)


				}

			*/
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
		conn, _ := ln.Accept() // Открываем порт

		go handleConnection(conn)

	}
}
