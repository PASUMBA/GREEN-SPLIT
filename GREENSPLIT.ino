// Green Split ESP32 Simple Dashboard (RECEIVER)
// * This sketch hosts a single, static web page that displays the fill levels
// * It now uses *Hardware Interrupts* for reliable signal detection.
// * Data Model: Bin #1 tracks composition counts (P, O, M) and a total fill level (0-100%).
// Incoming pulses increment the specific material count and the total fill level.
// * Wi-Fi: Madhav / 123456789
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --- Configuration ---
const char* ssid = "Madhav";
const char* password = "123456789";

// Server Setup
WebServer server(80);

// Pin Definitions for reading signals from the AI device (SSCMA outputs)
// IMPORTANT: Ensure these pins are correct for your ESP32 board.
const int PLASTIC_PIN = 4;   // Input pin for Plastic (Class 0) signal - Connect to SSCMA D0
const int ORGANIC_PIN = 2;  // Input pin for Organic (Class 1) signal - Connect to SSCMA D1
const int METAL_PIN = 15;    // Input pin for Metal (Class 2) signal - Connect to SSCMA D2

// Debounce Configuration
// We set this to 2 seconds (2000 ms) to ensure the 5-second pulse is only counted once,
// and we ignore any transient noise during the pulse.
const unsigned long DEBOUNCE_DELAY_MS = 2000; 

// --- INTERNAL STATE (MUST be volatile for use in Interrupts) ---
volatile int bin1_plastic_comp_count = 0; 
volatile int bin1_organic_comp_count = 0; 
volatile int bin1_metal_comp_count = 0;   
volatile int bin1_total_fill_level = 0;  

// State variables for non-blocking debouncing
volatile unsigned long lastPlasticDetectTime = 0;
volatile unsigned long lastOrganicDetectTime = 0;
volatile unsigned long lastMetalDetectTime = 0;

// The amount of fill capacity one item takes up (e.g., 5% capacity consumed per item)
const int FILL_INCREMENT_PER_ITEM = 5; 

// --- INTERRUPT SERVICE ROUTINES (ISRs) ---
// These functions run immediately when a rising voltage edge is detected.

void IRAM_ATTR detectPlastic() {
    unsigned long currentMillis = millis();
    // Check if enough time has passed since the last valid detection (debouncing)
    if (currentMillis - lastPlasticDetectTime > DEBOUNCE_DELAY_MS) {
        bin1_plastic_comp_count = min(100, bin1_plastic_comp_count + 1);
        bin1_total_fill_level = min(100, bin1_total_fill_level + FILL_INCREMENT_PER_ITEM);
        lastPlasticDetectTime = currentMillis; 
        // Note: Serial.print is avoided in ISRs as it can cause crashes, 
        // but we will print in the main loop for debugging.
    }
}

void IRAM_ATTR detectOrganic() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastOrganicDetectTime > DEBOUNCE_DELAY_MS) {
        bin1_organic_comp_count = min(100, bin1_organic_comp_count + 1);
        bin1_total_fill_level = min(100, bin1_total_fill_level + FILL_INCREMENT_PER_ITEM);
        lastOrganicDetectTime = currentMillis;
    }
}

void IRAM_ATTR detectMetal() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMetalDetectTime > DEBOUNCE_DELAY_MS) {
        bin1_metal_comp_count = min(100, bin1_metal_comp_count + 1);
        bin1_total_fill_level = min(100, bin1_total_fill_level + FILL_INCREMENT_PER_ITEM);
        lastMetalDetectTime = currentMillis;
    }
}

// --- C++ Raw String Literal for HTML Dashboard (Unchanged) ---
const char* DASHBOARD_HTML_TEMPLATE = R"html_content(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Green Split - Live Monitoring</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        :root {
            --color-primary: #10B981; /* Emerald Green */
        }
        body {
            font-family: 'Inter', sans-serif;
            background-color: #f4f6f8;
            min-height: 100vh;
        }
    </style>
</head>
<body class="p-4 sm:p-8">

    <div class="max-w-6xl mx-auto">
        <header class="text-center mb-10">
            <h1 class="text-4xl font-extrabold text-gray-800">Green Split Monitoring Dashboard</h1>
            <p class="text-lg text-gray-500 mt-2">Live Fill Levels for 10 Dump Bins</p>
        </header>

        <!-- FEATURED BIN 1 -->
        <div id="featured-bin-container" class="mb-8">
            <!-- Bin 1 will be rendered here prominently -->
        </div>

        <!-- REMAINING BINS (2-10) -->
        <div id="bins-container" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
            <!-- Bin cards 2-10 will be rendered here by JavaScript -->
        </div>
    </div>

    <script>
        // --- DATA STRUCTURES ---
        // Defines the 10 bins and their categories
        const BIN_MOCK_DATA = [
            { id: 1, material: "Mixed", color: "gray" }, // Bin 1 is now Mixed, tracking composition
            { id: 2, material: "Plastic", color: "blue" },
            { id: 3, material: "Plastic", color: "blue" },
            
            { id: 4, material: "Organic", color: "green" },
            { id: 5, material: "Organic", color: "green" },
            { id: 6, material: "Organic", color: "green" },
            { id: 7, material: "Organic", color: "green" },
            
            { id: 8, material: "Metal", color: "yellow" },
            { id: 9, material: "Metal", color: "yellow" },
            
            { id: 10, material: "Mixed", color: "gray" }
        ];

        let currentBin1Data = {
            plasticCompCount: 0,
            organicCompCount: 0,
            metalCompCount: 0,
            bin1TotalFill: 0
        };

        const binsContainer = document.getElementById('bins-container');
        const featuredBinContainer = document.getElementById('featured-bin-container');

        // --- UTILITY FUNCTIONS ---

        /** Get status color and text color based on fill level */
        function getStatusClasses(value, colorBase) {
            if (value >= 90) return 'bg-red-500 text-red-500'; 
            if (value >= 70) return 'bg-yellow-500 text-yellow-500';
            return 'bg-' + colorBase + '-500 text-' + colorBase + '-500';
        }

        /** Fetches current bin data from the ESP32 API */
        async function fetchBinData() {
            try {
                const response = await fetch('/api/data');
                currentBin1Data = await response.json();
                renderAllBins();
            } catch (error) {
                console.error('Failed to fetch bin data:', error);
            }
        }
        
        /** Renders all 10 bin cards, featuring Bin #1 prominently */
        function renderAllBins() {
            // --- BIN #1 COMPOSITION CALCULATIONS ---
            const P_count = currentBin1Data.plasticCompCount;
            const O_count = currentBin1Data.organicCompCount;
            const M_count = currentBin1Data.metalCompCount;
            const totalCount = P_count + O_count + M_count;
            const totalFill = currentBin1Data.bin1TotalFill;
            
            // Calculate composition percentages (handle division by zero if bin is empty)
            const P_pct = totalCount > 0 ? (P_count / totalCount * 100).toFixed(0) : 0;
            const O_pct = totalCount > 0 ? (O_count / totalCount * 100).toFixed(0) : 0;
            // The remaining metal percentage is calculated to ensure the total is 100%
            let M_pct = 100 - P_pct - O_pct;
            if (totalCount === 0) M_pct = 0; // If nothing is in the bin, comp is 0

            const featuredColor = 'gray'; // Bin 1 is now generally mixed/monitored
            const featuredStatusClasses = getStatusClasses(totalFill, featuredColor);
            const featuredProgressColorClass = featuredStatusClasses.split(' ')[0];
            const featuredTextColorClass = totalFill >= 90 ? 'text-red-600' : 'text-gray-800';
            
            // --- Render Featured Bin #1 (Larger Card with Composition Breakdown) ---
            const featuredHtml = '<div class="bg-white p-8 rounded-xl shadow-2xl border-l-8 border-' + featuredColor + '-500">' +
                '<h2 class="text-2xl font-extrabold text-gray-800 mb-6">Featured Bin: Dump Bin #1 (Real-time Input)</h2>' +
                '<div class="grid grid-cols-1 md:grid-cols-3 gap-6 items-center">' +
                    // Section 1: Total Fill Level
                    '<div>' +
                    '<p class="text-xl font-semibold text-gray-600">Overall Level</p>' +
                    '<p class="text-5xl font-extrabold ' + featuredTextColorClass + ' mt-1">' + totalFill + '%</p>' +
                    '</div>' +
                    // Section 2: Composition Status
                    '<div>' +
                    '<p class="text-xl font-semibold text-gray-600">Overall Status</p>' +
                    '<span class="inline-block px-4 py-1 mt-1 text-sm font-bold text-white rounded-full ' + featuredProgressColorClass + '">' + (totalFill >= 90 ? 'CRITICAL' : (totalFill >= 70 ? 'HIGH' : 'NORMAL')) + '</span>' +
                    '</div>' +
                    // Section 3: Total Items Detected (for context)
                    '<div>' +
                    '<p class="text-xl font-semibold text-gray-600">Items Detected</p>' +
                    '<p class="text-3xl font-extrabold text-gray-700 mt-1">' + totalCount + '</p>' +
                    '</div>' +
                '</div>' +
                
                // --- Composition Breakdown Chart ---
                '<div class="mt-8">' +
                    '<p class="text-lg font-semibold text-gray-700 mb-3">Material Composition Percentage</p>' +
                    '<div class="space-y-3">' +
                        // Plastic Composition
                        '<div>' +
                        '<div class="flex justify-between text-sm font-medium mb-1">' +
                        '<span>Plastic</span>' +
                        '<span>' + P_pct + '%</span>' +
                        '</div>' +
                        '<div class="w-full bg-gray-200 rounded-full h-3">' +
                        '<div class="h-3 rounded-full bg-blue-500 transition-all duration-700" style="width: ' + P_pct + '%;"></div>' +
                        '</div>' +
                        '</div>' +
                        // Organic Composition
                        '<div>' +
                        '<div class="flex justify-between text-sm font-medium mb-1">' +
                        '<span>Organic</span>' +
                        '<span>' + O_pct + '%</span>' +
                        '</div>' +
                        '<div class="w-full bg-gray-200 rounded-full h-3">' +
                        '<div class="h-3 rounded-full bg-green-500 transition-all duration-700" style="width: ' + O_pct + '%;"></div>' +
                        '</div>' +
                        '</div>' +
                        // Metal Composition
                        '<div>' +
                        '<div class="flex justify-between text-sm font-medium mb-1">' +
                        '<span>Metal</span>' +
                        '<span>' + M_pct + '%</span>' +
                        '</div>' +
                        '<div class="w-full bg-gray-200 rounded-full h-3">' +
                        '<div class="h-3 rounded-full bg-yellow-500 transition-all duration-700" style="width: ' + M_pct + '%;"></div>' +
                        '</div>' +
                        '</div>' +
                    '</div>' +
                '</div>' +
                
                // Progress Bar (Total Level)
                '<div class="mt-8">' +
                    '<p class="text-lg font-semibold text-gray-700 mb-2">Current Level Indicator</p>' +
                    '<div class="w-full bg-gray-200 rounded-full h-6">' +
                    '<div class="h-6 rounded-full transition-all duration-700 ' + featuredProgressColorClass + '" style="width: ' + totalFill + '%"></div>' +
                    '</div>' +
                '</div>' +
            '</div>';
            featuredBinContainer.innerHTML = featuredHtml;


            // --- Render Remaining Bins (Grid - showing Bin 1's total fill as mock data) ---
            const remainingBinsHtml = BIN_MOCK_DATA.slice(1).map(bin => {
                // All other bins show Bin #1's total fill level as a stand-in
                const value = totalFill; 
                const colorBase = bin.color; // Use their designated color
                
                const statusClasses = getStatusClasses(value, colorBase);
                const progressColorClass = statusClasses.split(' ')[0];
                const textColorClass = value >= 90 ? 'text-red-600' : 'text-gray-800';

                return '<div class="bg-white p-6 rounded-xl shadow-lg border-t-4 border-' + colorBase + '-500">' +
                    '<h3 class="text-xl font-bold ' + textColorClass + ' mb-2">Bin #' + bin.id + '</h3>' +
                    '<p class="text-sm font-medium text-gray-500">Waste Type: ' + bin.material + '</p>' +
                    
                    '<div class="mt-4">' +
                        '<div class="text-lg font-extrabold ' + textColorClass + ' mb-1 flex justify-between items-center">' +
                            '<span>Fill Level:</span>' +
                            '<span>' + value + '%</span>' +
                        '</div>' +
                        
                        // Progress Bar
                        '<div class="w-full bg-gray-200 rounded-full h-4">' +
                            '<div class="h-4 rounded-full transition-all duration-700 ' + progressColorClass + '" style="width: ' + value + '%"></div>' +
                        '</div>' +
                    '</div>' +

                '</div>';
            }).join('');

            binsContainer.innerHTML = remainingBinsHtml;
        }

        /** Starts the continuous data fetching loop */
        let pollingInterval = null;
        function startDataPolling() {
            if (pollingInterval) clearInterval(pollingInterval);
            // Fetch data and re-render every 2 seconds
            pollingInterval = setInterval(fetchBinData, 2000); 
        }

        // --- Initialization ---

        // Start fetching data and rendering once the DOM is fully loaded
        window.addEventListener('DOMContentLoaded', () => {
            fetchBinData(); // Initial render
            startDataPolling(); 
        });

    </script>
</body>
</html>
)html_content";

// --- Functions to handle Web Server Requests (Unchanged) ---

/** Handle root path / */
void handleRoot() {
  server.send(200, "text/html", DASHBOARD_HTML_TEMPLATE);
  Serial.println("Client requested dashboard page.");
}

/** Handle JSON API request for current data */
void handleApiData() {
  // We read the volatile variables here.
  String json = "{\"plasticCompCount\": " + String(bin1_plastic_comp_count) + 
                ", \"organicCompCount\": " + String(bin1_organic_comp_count) + 
                ", \"metalCompCount\": " + String(bin1_metal_comp_count) + 
                ", \"bin1TotalFill\": " + String(bin1_total_fill_level) + "}";

  server.send(200, "application/json", json);
}

/** Handle JSON API request to update data (Kept for API robustness) */
void handleApiUpdate() {
    // This handler can be used to reset or manually set fill levels if needed
    if (server.method() != HTTP_POST || !server.hasArg("plain")) {
        server.send(400, "text/plain", "Bad Request");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Bad Request: Invalid JSON");
        return;
    }

    int target = doc["target"];
    int value = doc["value"];
    
    // Example: If target 99 is sent, reset the whole bin
    if (target == 99 && value == 0) {
        bin1_plastic_comp_count = 0;
        bin1_organic_comp_count = 0;
        bin1_metal_comp_count = 0;
        bin1_total_fill_level = 0;
        Serial.println("API commanded full bin reset.");
    }
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

/** Not Found Handler */
void handleNotFound() {
    server.send(404, "text/plain", "404: Not Found");
}

// --- Debugging Function ---

// Use this to monitor the state of the volatile counters in the main loop
void printStateForDebug() {
    static unsigned long lastPrintTime = 0;
    const unsigned long printInterval = 2000; // Print every 2 seconds

    if (millis() - lastPrintTime >= printInterval) {
        Serial.printf("Current State: P=%d, O=%d, M=%d | Total Fill: %d%%\n", 
                      bin1_plastic_comp_count, 
                      bin1_organic_comp_count, 
                      bin1_metal_comp_count, 
                      bin1_total_fill_level);
        lastPrintTime = millis();
    }
}


// --- Setup and Loop ---

void setup() {
    Serial.begin(115200);

    // Initialize input pins
    // CRITICAL: Use INPUT_PULLDOWN to prevent floating pins and phantom reads
    pinMode(PLASTIC_PIN, INPUT_PULLDOWN);
    pinMode(ORGANIC_PIN, INPUT_PULLDOWN);
    pinMode(METAL_PIN, INPUT_PULLDOWN);
    
    // ** ATTACH HARDWARE INTERRUPTS **
    // The RISING setting ensures the ISR runs the moment the SSCMA switches the pin from LOW to HIGH.
    attachInterrupt(digitalPinToInterrupt(PLASTIC_PIN), detectPlastic, RISING);
    attachInterrupt(digitalPinToInterrupt(ORGANIC_PIN), detectOrganic, RISING);
    attachInterrupt(digitalPinToInterrupt(METAL_PIN), detectMetal, RISING);

    // Connect to Wi-Fi
    Serial.printf("Connecting to %s...", ssid);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.print("Access Dashboard at: http://");
        Serial.println(WiFi.localIP());

        // Setup server routes
        server.on("/", handleRoot);
        server.on("/api/data", handleApiData);
        server.on("/api/update", handleApiUpdate);
        server.onNotFound(handleNotFound);

        server.begin();
        Serial.println("HTTP server started.");
    } else {
        Serial.println("");
        Serial.println("Failed to connect to WiFi. Please check power and antenna.");
    }
}

void loop() {
    server.handleClient();
    printStateForDebug(); // Monitor the counters without blocking the loop
    delay(5); 
}