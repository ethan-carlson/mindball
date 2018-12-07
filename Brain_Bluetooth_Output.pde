import processing.net.*;
import processing.serial.*;

Serial port;
  
PrintWriter output;
JSONObject json;

Client myClient; 
String scaleMode;

//math variables
float alphaComposite;
float betaComposite;
int gameRatio;
float thetaLog;
float lowAlphaLog;
float highAlphaLog;
float lowBetaLog;
float highBetaLog;
int MARatio;
float attention;
float meditation;

//change to 'true' to output data to text file
boolean writeToFile = false;
//change to 'true' to send data to the Arduino
boolean gameOn = true;

void setup() {
  size(200, 200);

    //option to write data to text file
  if (writeToFile){
    output = createWriter("data.txt");
  }
  if (gameOn){
    //may need to change serial port to Arduino bluetooth module
    port = new Serial(this, Serial.list()[1], 9600);
  }
  
  // Connect to ThinkGear socket (default = 127.0.0.1:13854)
  String thinkgearHost = "127.0.0.1";
  int thinkgearPort = 13854;
  
  String envHost = System.getenv("THINKGEAR_HOST");
  if (envHost != null) {
    thinkgearHost = envHost;
  }
  String envPort = System.getenv("THINKGEAR_PORT");
  if (envPort != null) {
     thinkgearPort = Integer.parseInt(envPort);
  }
 
  println("Connecting to host = " + thinkgearHost + ", port = " + thinkgearPort);
  myClient = new Client(this, thinkgearHost, thinkgearPort);
  String command = "{\"enableRawOutput\": false, \"format\": \"Json\"}\n";
  print("Sending command");
  println (command);
  myClient.write(command);

}

void draw() {
  //comment
}

void clientEvent(Client  myClient) {
  
  // Sample JSON data:
  // {"eSense":{"attention":91,"meditation":41},"eegPower":{"delta":1105014,"theta":211310,"lowAlpha":7730,"highAlpha":68568,"lowBeta":12949,"highBeta":47455,"lowGamma":55770,"highGamma":28247},"poorSignalLevel":0}
  
  if (myClient.available() > 0) {
  
    String data = myClient.readString();
    json = parseJSONObject(data);
    
    try {      
      JSONObject esense = json.getJSONObject("eSense");
      if (esense != null) {
        attention = esense.getInt("attention");
        meditation = esense.getInt("meditation");
        
        println("Attention: " + attention);
        println("Meditation: " + meditation);
        
        MARatio = int(100*(meditation/attention));
        println("MA Ratio: " + MARatio);
        
        /*
        if (writeToFile){
          output.println("Attention: " + attention);
          output.println("Meditation: " + meditation);
        }
        */
      }
      
      JSONObject eegPower = json.getJSONObject("eegPower");
      if (eegPower != null) {
        
        //take the log of wave values to allow for comparison
        thetaLog = log(eegPower.getInt("theta"));
        lowAlphaLog = log(eegPower.getInt("lowAlpha"));
        highAlphaLog = log(eegPower.getInt("highAlpha"));
        lowBetaLog = log(eegPower.getInt("lowBeta"));
        highBetaLog = log(eegPower.getInt("highBeta"));
        
        //create relaxation and focus composites
        alphaComposite = lowAlphaLog + highAlphaLog;
        betaComposite = lowBetaLog + highBetaLog;
        
        //value to send out to the game
        gameRatio = int(100*(alphaComposite/betaComposite));
        
        //println("A-Comp: " + alphaComposite);
        //println("B-Comp: " + betaComposite);
        println("Game Ratio: " + gameRatio);
        
        if (writeToFile){
          //output.println("EEG Power Values:");
          output.print(eegPower.getInt("delta") + ", ");
          output.print(eegPower.getInt("theta") + ", ");
          output.print(eegPower.getInt("lowAlpha") + ", ");
          output.print(eegPower.getInt("highAlpha") + ", ");
          output.print(eegPower.getInt("lowBeta") + ", ");
          output.print(eegPower.getInt("highBeta") + ", ");
          output.print(eegPower.getInt("lowGamma") + ", ");
          output.print(eegPower.getInt("highGamma") + ", ");
          output.println("");
        }
        
        if (gameOn){
          port.write(gameRatio);
        }
      }
    }   
    catch (Exception e) {
      println ("There was an error parsing the JSONObject." + e);
    }
  }
}

//need to be able to close out the data file
void keyPressed() {
  if (writeToFile){
    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
  }
  exit(); // Stops the program
}
