const server = require('http').createServer();
const io = require('socket.io')(server);

const users = require("./users.json")
const isValidUser = (username, password) => {
    return users.find(user => user.username === username && user.password === password)
}
const getUser = (username) => {
    return users.find(user => user.username === username)
}

io.on('connection', client => {
    client.loggedIn = false;

    console.log(`Client connected [id=${client.id}]`);

    client.on('login', data => {
        console.log(`Client sent login data:`, data);

        if (isValidUser(data.username, data.password)) {
            console.log(`Client logged in [id=${client.id}]`);
            client.loggedIn = true;
            client.username = data.username;
            client.emit('login', { success: true, userData: getUser(data.username) });
        } else {
            console.log(`Client failed to log in [id=${client.id}]`);
            client.emit('login', { success: false });
        }
    });

    client.on('disconnect', () => {
        console.log(`Client disconnected [id=${client.id}]`);
    });
});

server.listen(3000, 'localhost', () => {
    console.log(`Server is running on http://localhost:3000`);
});
