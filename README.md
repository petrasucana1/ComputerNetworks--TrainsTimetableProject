# **Concurrent Server Application for Real-Time Railway Timetable Management**

This project focuses on the development of a **server-client system** designed for **real-time railway timetable management**, where the server can handle multiple clients simultaneously. The main goal of the system is to provide real-time information on **train schedules**, including **departure/arrival times**, and updates on **delays**. Built using **C programming language**, this application leverages **multithreaded programming**, **socket communication**, and **XML data handling** to ensure smooth interaction between server and clients.

---

## **Project Overview:**
The application extracts data from **XML files** that store the railway timetable information. Upon receiving a client request, the server provides details about the **train schedules** for the current day and specific information such as the status of **departures** and **arrivals** in the next hour. Clients can also report **train delays**, specifying the **train number**, **delay interval**, and an **estimated arrival time**.

---

## **Key Features:**
### **Concurrency Management**  
The server is designed to handle multiple clients at the same time using **threads**. Each client is assigned a thread to ensure smooth real-time communication without blocking other users.

### **Command Design Pattern**  
The **Command design pattern** is implemented to handle client commands like **train schedules**, **departures**, **arrivals**, and **delays** in a structured and scalable way.

### **Real-time Updates**  
The server continuously updates train delays based on information received from the clients. It processes commands efficiently, maintaining an organized flow of data.

### **TCP/IP Socket Communication**  
The server and clients communicate through **TCP/IP sockets**, ensuring reliable, bidirectional data transmission.

### **XML Data Parsing**  
The server parses XML files containing the **train timetable data** using the **libxml2** library, making it easy to read and manipulate the information.

---

## **Technical Details:**
The server uses **multithreaded execution**, where each client interaction is managed by a separate thread, allowing for efficient processing of multiple requests simultaneously. **Sockets** are used to establish communication channels between the server and clients. Each client’s request is placed in a **command queue**, ensuring synchronization and preventing overload.

The server reads data from **XML files**, storing it in structures such as **TrainStations** (representing stations) and **TrainsDataBase** (storing train details). Commands are processed in the queue and responses are sent back to the client in real-time.

---

## **Technologies Applied:**
### **Multithreaded Programming (Threads)**  
To handle multiple clients efficiently by creating individual threads for each client.

### **Sockets**  
Ensures flexible and reliable communication between the server and clients.

### **XML Parsing (libxml2)**  
To efficiently parse and manipulate train schedule data stored in **XML format**.

### **Command Design Pattern**  
Helps organize client requests, allowing for easy management and extension of system functionality.

---

## **Real-World Application Scenarios:**
The system is intended for use in **real-world railway operations**. For example, a user can connect to the application, submit their **location (station)**, and the system will provide **real-time train schedules** for that location. Additionally, users can report delays, which will be displayed to other users connected to the system.

---

## **Potential Enhancements:**
While the current system is functional, it can be enhanced by:
- **Integrating data from official sources** to ensure accuracy in real-time train schedules.
- **Adding a graphical interface** to make the application more user-friendly.
- Introducing features like **ticket purchasing** and **user authentication** (login/logout) to extend its functionality.
