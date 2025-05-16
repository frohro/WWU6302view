#include <Six302.h>
#include <math.h>

CommManager cm(5000, 50000);

float t, output;

void setup() {
   // Example plot
   cm.addPlot(&output, "Plot", -1.1, 1.1);
   // Connect via serial
   cm.connect("Frohne-2.4GHz", NULL);
}

void loop() {
   output = sin(t = t + 0.02);
   cm.step();
}