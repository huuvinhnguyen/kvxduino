/*
  This a a demo webpage trying to reproduce the arduino website look
  You will need the arduino ethernet shield

  made by JO3RI www.JO3RI.be/arduino
*/

//#include <SPI.h>
//#include <Ethernet.h>

// This is the hardware address of the Arduino on your network.
// You won't need to change this. Learn more here.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// This is the tcp/ip address of the Arduino on your network.
// You will access the Arduino by typing it in a web browser like
// this: http://192.168.1.177 Learn more here.
// You might want to change this to be the same as your computer's
// IP address except for the last number. Learn more here.
byte ip[] = { 192, 168, 14, 85};

// Create a server named webserver listening on port 80 (http). You
// don't want to change the port number. Learn more about ports or
// the server class.
Server webserver(80);

// Functions that are run once when the Arduino is started
void setup()
{

  // Bring the Arduino on the network using the MAC and IP we
  //specified before. Learn more about Ethernet.begin().
  Ethernet.begin(mac, ip);
  // Start a server on the port we specified before. Learn more
  // about server.begin().
  webserver.begin();

}

// Repeat these functions as long as the Arduino is powered on
void loop()
{

  // Create a client named webclient listening for connections to the
  // server. Learn more about Client or server.available()
  Client webclient = webserver.available();

  // If someone connects to the server...
  if (webclient) {

    // Create a variable to hold whether or not we have received a blank
    // line from the web browser
    boolean current_line_is_blank = true;

    // Run the following code as long as the client remains connected
    // Learn more about client.connected()
    while (webclient.connected()) {

      // If the client has sent us some data...
      // Learn more about client.available()
      if (webclient.available()) {

        // Keep the last letter of whatever they sent us
        // Learn more about client.read()
        char c = webclient.read();

        // If we've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so we can send a reply
        if (c == 'n' && current_line_is_blank) {

          // Send a basic HTTP response header (the blank line at the end
          // is required). Learn more about client.println()
          webclient.println("HTTP/1.1 200 OK");
          webclient.println("Content-Type: text/html");
          webclient.println();

          // Now send whatever html you want to appear on the web page
          webclient.println("<html><body marginwidth\"0\" marginheight=\"0\" topmargin=\"0\" leftmargin=\"0\" style=\"margin: 0; padding: 0;\">");
          webclient.println("<table bgcolor=\"#999999\" border=\"0\" width=\"100%\" cellpadding=\"1\">");
          webclient.println("<tr><td><font color=\"white\" size=\"2\" face=\"Verdana\">&nbsp Arduino Ethernet Shield setup page</font></td></tr></table><br>");
          webclient.println("<table border=\"0\" width=\"100%\"><tr><td width=110px>&nbsp</td><td width=200px><style>pre {font-size:8pt; letter-spacing:2px; line-height:8pt; font-weight:bold;}</style>");
          webclient.println("<pre><font color=\"#00979d\">");
          webclient.println("    ###      ###   TM");
          webclient.println("  ##   ##  ##   ##  ");
          webclient.println(" ##     ####  #  ## ");
          webclient.println(" #  ###  ##  ### ## ");
          webclient.println(" ##     ####  #  ## ");
          webclient.println("  ##   ##  ##   ##  ");
          webclient.println("   #####    #####   ");
          webclient.println("</font></pre></td><td>&nbsp</td></tr><tr><td width=110px>&nbsp</td>");
          webclient.println("<td width=200px><font color=\"#00979d\" size=\"6\" face=\"Verdana\"><strong> ARDUINO</strong></font></td><td>&nbsp</td></tr></table><br>");
          webclient.println("<table bgcolor=\"#00979d\" border=\"0\" width=\"100%\" cellpadding=\"3\"><tr><td width=105px></td>");
          webclient.println("<td width=120px><font color=\"white\" size=\"2\" face=\"Verdana\">Network info</font></td><td width=120px>");
          webclient.println("<font color=\"white\" size=\"2\" face=\"Verdana\">Network setup</font></td><td></td></tr></table></body></html>");

          break;

        }

        // If we received a new line character from the client ...
        if (c == 'n') {

          // Track that the line is blank
          current_line_is_blank = true;

          // If we don't receive a new line character and don't receive an r
        } else if (c != 'r') {

          // Track that we we got some data
          current_line_is_blank = false;

        }

      }

    }

    // Give the client some time to receive the page
    // Learn more about delay()
    delay(1);

    // Close the connection to the client now that we are finished
    // Learn more about client.stop()
    webclient.stop();

  }

}
