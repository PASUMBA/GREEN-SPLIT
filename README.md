# GREEN-SPLIT

🔍 Overview

The Smart Waste Segregation and Monitoring System is an IoT + AI-based solution designed to tackle the issue of overflowing trash bins and improper waste segregation in urban areas.

This project combines machine learning–powered waste detection, automated physical segregation, and a real-time monitoring website for authorities and citizens.


---

⚙ System Architecture

1. Hardware System – Smart Segregator

Core Components:
Grove Vision AI Module – for real-time image-based waste classification.
Seeed Studio Xiao ESP32S3 – for data processing and Wi-Fi connectivity.
Servo Motor – to rotate the waste bin section to direct each detected item into the correct bin.
Three Bins: for Plastic, Metal, and Organic waste.


Working Principle:

1. Waste is inserted into a main input bin.
2. The Grove Vision AI camera captures the image of the waste.
3. The machine learning model (trained on waste image dataset) classifies the item into one of three categories:
Plastic
Metal
Organic
4. The ESP32S3 microcontroller receives the classification output and rotates the servo motor to position the correct bin below the input chamber.
5. The waste is dropped automatically into the correct compartment.

ML Model:
Custom trained model using TensorFlow Lite.
Optimized for Grove Vision AI hardware inference.

2. Web-Based Monitoring Platform

🌐 Purpose

To help urban waste authorities monitor bin status, track segregation efficiency, and manage waste collection operations — all through an interactive dashboard.

💻 Features

1. Login System

Authority Login: for waste management officials.
Citizen Login: for public reporting.

2. Trash Map 

Displays all trash bins in the city as 3D cartoon-style icons.
Shows current fill level, waste composition (pie chart), and days left for collection when clicked.
Marks sewage overflows and illegal waste dumps reported by citizens.


3. Alerts Dashboard

Color-coded bins (🟢 Green / 🟡 Yellow / 🔴 Red) based on waste level.
Separate alert list for sewage overflow and illegal dumps.

4. Analysis Section

Graphs and charts comparing monthly waste accumulation and clearance.
Composition and trend analysis for data-driven insights.

5. Financial Records
Tracks amount and mass of segregated waste sold to companies (e.g., Northmans).
Displays monthly revenue charts and comparison data.



6. Citizen Reporting Portal

Citizens can report:
Sewage overflows
Illegal waste dumps
Upload images and pin locations on the map.
Reports appear in real-time on the authority dashboard with clickable photo previews.





---

3. Integration Between Hardware and Website

The ESP32S3 connects to the website backend via Wi-Fi (HTTP/MQTT protocol).

Each segregation cycle sends:

Type of waste detected.

Timestamp of disposal.

Sensor data (bin fill level, optional).


The web server updates the central database and visualizes this in the dashboard map.

This ensures the hardware system and online monitoring tool stay perfectly synchronized.



---

🧠 Tech Stack

Layer	Technology

Hardware	Grove Vision AI, Xiao ESP32S3, Servo Motor
Machine Learning	TensorFlow Lite, Edge Impulse / Custom Model
Firmware	Arduino IDE / ESP-IDF
Backend	Node.js / Flask (for API communication)
Frontend	HTML, CSS, JavaScript, React (or similar)
Database	Firebase / MongoDB
Visualization	Chart.js, Google Maps / Leaflet.js for map integration



---

🧩 Key Highlights

🔄 Fully Automated Segregation using AI-powered vision.

🗺 Real-Time Monitoring Dashboard for city waste management.

👥 Citizen Participation Portal for community-driven waste management.

💰 Waste-to-Revenue Tracking integrated with sales data visualization.

🌱 Data Analytics Dashboard for monthly insights and environmental impact tracking.



---

🚀 Future Enhancements

GSM alert module for remote areas without Wi-Fi.

Integration with municipal waste collection APIs.

Predictive analysis using historical waste data.

Collaboration with recycling companies for live data exchange.



---

📸 Demo Content

Preloaded synthetic data for demo bins and citizen reports.

Sample images of illegal dumps and sewage overflows.

Simulated waste level and composition data displayed on map.



---

👩‍💻 Team & Credits

Developed by: MADHAV K ANIL
TEAM: ME_
