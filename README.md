# 🔐 Secure Multiprocessor TCP Server

> A secure, concurrent client-server system built with **C** (server) and **Python** (client) using TCP socket programming. Developed as part of the Network Programming (IE2102) module at **SLIIT**.

---

## 📌 Overview

This project implements a custom TCP-based authentication server capable of handling multiple simultaneous clients using Unix process forking. It includes a structured communication protocol, user authentication, and several security hardening features.

| Component | Language | Description |
|-----------|----------|-------------|
| `server_1551.c` | C | Multiprocessing TCP server |
| `client_1551.py` | Python | Interactive TCP client |
| `Makefile_1551` | Make | Build automation |
| `users.txt` | Text | User credentials store |
| `server_IT24101551.log` | Log | Activity log file |

---

## ✨ Features

### 🔁 Multiprocessing (A2)
- Each client connection is handled by a separate **child process** using `fork()`
- Zombie processes are prevented using `waitpid()` with `WNOHANG`
- Supports up to 10 queued connections

### 🔑 Authentication System (A3)
| Command | Description |
|---------|-------------|
| `REGISTER <user> <pass>` | Creates a new user account |
| `LOGIN <user> <pass>` | Authenticates and returns a session token |
| `LOGOUT` | Ends the current session |

- Passwords are stored using a shift-based hash
- Session tokens are generated on successful login (`TKN<random>`)

### 🛡️ Security Features (A4)

| Feature | Description |
|---------|-------------|
| **Username Validation** | Only alphanumeric characters allowed |
| **Rate Limiting** | Max 5 requests/second per connection (`ERR 429`) |
| **Brute-Force Protection** | Account locked after 3 failed login attempts (`ERR 403`) |
| **Payload Validation** | Requests over 4096 bytes are rejected (`ERR 413`) |

### 📋 Logging System (A5)
Every client action is logged to `server_IT24101551.log` with:
- Timestamp
- Client IP and port
- Process ID (PID)
- Username
- Command issued
- Server response

---

## 📡 Custom Protocol (A1)

### Client Request Format
```
LEN:<length>
<COMMAND> <arg1> <arg2>
```

**Example:**
```
LEN:20
LOGIN user1 mypassword
```

### Server Response Format
```
# Success
OK 200 SID:1015 <message>

# Error
ERR <code> SID:1015 <message>
```

### Response Codes
| Code | Meaning |
|------|---------|
| `200` | Success |
| `400` | Bad request / User exists / Invalid username |
| `401` | Invalid credentials |
| `403` | Account locked |
| `413` | Payload too large |
| `429` | Too many requests |

---

## 🚀 Getting Started

### Prerequisites
- GCC compiler
- Python 3
- Linux/Unix environment

### Build & Run

```bash
# Clone the repository
git clone https://github.com/<your-username>/secure-tcp-server.git
cd secure-tcp-server

# Build the server
make -f Makefile_1551

# Start the server
./server_1551

# In another terminal, start the client
python3 client_1551.py
```

### Example Session
```
Enter message: REGISTER alice secret123
Server: OK 200 SID:1015 Registered

Enter message: LOGIN alice secret123
Server: OK 200 SID:1015 TOKEN:TKN45231 Login successful

Enter message: LOGOUT
Server: OK 200 SID:1015 Logged out
```

---

## 🧪 Testing

The following scenarios were tested and verified:

- ✅ Server startup and client connection
- ✅ User registration (new and duplicate)
- ✅ Login success with token generation
- ✅ Login failure with invalid credentials
- ✅ Account lockout after 3 failed attempts
- ✅ Multiple concurrent clients via multiprocessing
- ✅ Log file generation and accuracy
- ✅ Rate limiting enforcement
- ✅ Zombie process prevention

---

## 📁 Project Structure

```
.
├── server_1551.c           # Main server source code
├── client_1551.py          # Python client
├── Makefile_1551           # Build configuration
├── users.txt               # User database (auto-created)
└── server_IT24101551.log   # Activity log (auto-created)
```

---

## ⚠️ Known Limitations & Future Improvements

- [ ] Replace Caesar-cipher hash with SHA-256 + salt
- [ ] Add session timeout/expiry mechanism
- [ ] Implement TLS/SSL encryption for data in transit
- [ ] Persist rate-limit state across requests
- [ ] Add token validation for protected commands

---

## 📚 References

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [RFC 793 – Transmission Control Protocol](https://www.rfc-editor.org/rfc/rfc793)
- Linux Manual Pages (`man fork`, `man socket`, `man waitpid`)

---

## 👩‍💻 Author

**Kuyilini T** — IT24101551  
BSc (Hons) Information Technology  
Sri Lanka Institute of Information Technology (SLIIT)  
📅 April 2026

---

> *This project was developed for academic purposes as part of the IE2102 Network Programming module.*
