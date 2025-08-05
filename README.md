# chat
Pet-project: client-server application based on PostgreSQL as a db.
Technologies: C++ (Qt Framework for GUI), client requests via TCP sockets, interaction with PostgreSQL, data exchange in JSON format, graphical interface on Qt Widgets
Actions that are available: registration and authentication users, creating personal and group chats, adding and removing users from chats (for admin), sending real-time massages, viewing the history of chats
General: this project consists from 2 parts: server and client. Client connects to the server. All actions are transmitted as JSON requests. The server processes the request, interacts with PostgreSQL, and returns a response. The client updates the interface.
