// SSCMA Material Detection with 5-Second Output Pulses (Plastic, Organic, Metal)
#include <Seeed_Arduino_SSCMA.h>

SSCMA AI;

// Define the digital pins for clarity and the new material targets
const int PLASTIC_PIN = D0;
const int ORGANIC_PIN = D1;
const int METAL_PIN = D2;

void setup()
{
    // Initialize serial communication
    Serial.begin(9600);
    while (!Serial); // Wait for serial port to connect (useful for debugging)

    // Initialize the AI module
    Serial.println("Initializing SSCMA AI...");
    if (AI.begin()) {
        Serial.println("SSCMA AI failed to initialize!");
        // Optionally hang or indicate error
    } else {
        Serial.println("SSCMA AI initialized successfully.");
    }

    // Set digital pins as OUTPUT
    pinMode(PLASTIC_PIN, OUTPUT);
    pinMode(ORGANIC_PIN, OUTPUT);
    pinMode(METAL_PIN, OUTPUT);

    // Ensure all pins are initially LOW for a clean state
    digitalWrite(PLASTIC_PIN, LOW);
    digitalWrite(ORGANIC_PIN, LOW);
    digitalWrite(METAL_PIN, LOW);
    Serial.println("Digital output pins D0 (Plastic), D1 (Organic), D2 (Metal) set to LOW.");
}

void loop()
{
    // Attempt to run inference
    if (!AI.invoke())
    {
        Serial.println("----------------------------------------");
        Serial.println("--- New Inference Cycle: Invoke Success ---");
        
        // Print performance statistics
        Serial.print("Perf: Preprocess=");
        Serial.print(AI.perf().prepocess);
        Serial.print("ms, Inference=");
        Serial.print(AI.perf().inference);
        Serial.print("ms, Postprocess=");
        Serial.println(AI.perf().postprocess);

        bool actionTaken = false;

        // Iterate through all detected classes/targets
        for (int i = 0; i < AI.classes().size(); i++)
        {
            int target = AI.classes()[i].target;
            int score = AI.classes()[i].score;

            Serial.print("Class[");
            Serial.print(i);
            Serial.print("]: Target=");
            Serial.print(target);
            Serial.print(", Score=");
            Serial.println(score);
            
            // --- Detection Logic for 5-Second Pulse ---
            
            // Check for Plastic (target 0, score > 75)
            if (target == 0 && score > 75)
            {
                Serial.println(">>> ACTION: Plastic (Target 0) detected! D0 HIGH for 5 seconds.");
                
                // Set D0 HIGH and ensure others are LOW (to balance all pins)
                digitalWrite(PLASTIC_PIN, HIGH);
                digitalWrite(ORGANIC_PIN, LOW);
                digitalWrite(METAL_PIN, LOW);
                
                delay(5000); // Wait for 5 seconds (This blocks the loop)
                
                // Reset D0 back to LOW after the pulse
                digitalWrite(PLASTIC_PIN, LOW);
                Serial.println(">>> ACTION COMPLETE: D0 set back to LOW.");
                actionTaken = true;
                break; // Stop processing other classes in this frame.
            }

            // Check for Organic (target 1, score > 75)
            else if (target == 1 && score > 75)
            {
                Serial.println(">>> ACTION: Organic (Target 1) detected! D1 HIGH for 5 seconds.");
                
                // Set D1 HIGH and ensure others are LOW (to balance all pins)
                digitalWrite(PLASTIC_PIN, LOW);
                digitalWrite(ORGANIC_PIN, HIGH);
                digitalWrite(METAL_PIN, LOW);
                
                delay(5000); // Wait for 5 seconds (This blocks the loop)
                
                // Reset D1 back to LOW after the pulse
                digitalWrite(ORGANIC_PIN, LOW);
                Serial.println(">>> ACTION COMPLETE: D1 set back to LOW.");
                actionTaken = true;
                break; // Stop processing other classes in this frame.
            }

            // Check for Metal (target 2, score > 50)
            else if (target == 2 && score > 50)
            {
                Serial.println(">>> ACTION: Metal (Target 2) detected! D2 HIGH for 5 seconds.");
                
                // Set D2 HIGH and ensure others are LOW (to balance all pins)
                digitalWrite(PLASTIC_PIN, LOW);
                digitalWrite(ORGANIC_PIN, LOW);
                digitalWrite(METAL_PIN, HIGH);
                
                delay(5000); // Wait for 5 seconds (This blocks the loop)
                
                // Reset D2 back to LOW after the pulse
                digitalWrite(METAL_PIN, LOW);
                Serial.println(">>> ACTION COMPLETE: D2 set back to LOW.");
                actionTaken = true;
                break; // Stop processing other classes in this frame.
            }
        }
        
        if (!actionTaken) {
            // If no high-confidence target was found, ensure all pins are LOW.
             digitalWrite(PLASTIC_PIN, LOW);
             digitalWrite(ORGANIC_PIN, LOW);
             digitalWrite(METAL_PIN, LOW);
             Serial.println("No high-confidence action criteria met. All pins remain LOW.");
        }

        // --- Debug Printouts for other outputs (Boxes, Points, Keypoints) ---
        Serial.println("--- Full AI Output Data ---");

        // Print outputs for debugging (boxes)
        for (int i = 0; i < AI.boxes().size(); i++)
        {
            Serial.print("Box[");
            Serial.print(i);
            Serial.print("] target=");
            Serial.print(AI.boxes()[i].target);
            Serial.print(", score=");
            Serial.print(AI.boxes()[i].score);
            Serial.print(", x=");
            Serial.print(AI.boxes()[i].x);
            Serial.print(", y=");
            Serial.print(AI.boxes()[i].y);
            Serial.print(", w=");
            Serial.print(AI.boxes()[i].w);
            Serial.print(", h=");
            Serial.println(AI.boxes()[i].h);
        }

        // Print outputs for debugging (points)
        for (int i = 0; i < AI.points().size(); i++)
        {
            Serial.print("Point[");
            Serial.print(i);
            Serial.print("]: target=");
            Serial.print(AI.points()[i].target);
            Serial.print(", score=");
            Serial.print(AI.points()[i].score);
            Serial.print(", x=");
            Serial.print(AI.points()[i].x);
            Serial.print(", y=");
            Serial.println(AI.points()[i].y);
        }

        // Print outputs for debugging (keypoints)
        for (int i = 0; i < AI.keypoints().size(); i++)
        {
            Serial.print("keypoint[");
            Serial.print(i);
            Serial.print("] target=");
            Serial.print(AI.keypoints()[i].box.target);
            Serial.print(", score=");
            Serial.print(AI.keypoints()[i].box.score);
            Serial.print(", box:[x=");
            Serial.print(AI.keypoints()[i].box.x);
            Serial.print(", y=");
            Serial.print(AI.keypoints()[i].box.y);
            Serial.print(", w=");
            Serial.print(AI.keypoints()[i].box.w);
            Serial.print(", h=");
            Serial.print(AI.keypoints()[i].box.h);
            Serial.print("], points:[");

            for (int j = 0; j < AI.keypoints()[i].points.size(); j++)
            {
                Serial.print("[");
                Serial.print(AI.keypoints()[i].points[j].x);
                Serial.print(",");
                Serial.print(AI.keypoints()[i].points[j].y);
                Serial.print("],");
            }
            Serial.println("]");
        }
    } else {
        Serial.println("ERROR: AI.invoke() failed. Check camera connection or model.");
    }
}