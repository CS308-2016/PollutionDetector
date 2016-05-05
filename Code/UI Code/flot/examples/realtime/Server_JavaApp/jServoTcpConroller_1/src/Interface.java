/**
 *
 * @author secakmak
 */
import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.io.DataInputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

public class Interface extends javax.swing.JDialog 
{
    public ServerSocket serverSocket;
    public Socket server;
    public DataOutputStream out;
    
    public Interface(java.awt.Frame parent, boolean modal) 
    {
        super(parent, modal);
        initComponents();
        
        OpenServer();
         
    }
    
    public void OpenServer()
    {
        try
        {
            serverSocket = new ServerSocket(9999);
        }
        catch(IOException e)
        {
            System.out.println("Error");
        }
        
        new Thread(new Runnable() 
        { 
            public void run() 
            { 
                try 
                {
                    server = serverSocket.accept();
                    jLabel1.setText("Client Info:"+server.getRemoteSocketAddress());
                    
                    out = new DataOutputStream(server.getOutputStream());
                    PrintWriter pw = new PrintWriter(out, true);
			pw.println("What's you name?");
                        
                        while(true) {

                            BufferedReader br = new BufferedReader(new InputStreamReader(server.getInputStream()));
                            String str = br.readLine();
                            String[] splited = str.split("\\s+");
                            splited[0] = splited[0] + "\n";
                            if (splited[1].equals("1")) {
                                try {
                                    Files.write(Paths.get("data.txt"), splited[0].getBytes(), StandardOpenOption.APPEND);
                                }catch (IOException e) {
                                    //exception handling left as an exercise for the reader
                                }
                            }
                            else {
                                try {
                                    Files.write(Paths.get("data1.txt"), splited[0].getBytes(), StandardOpenOption.APPEND);
                                }catch (IOException e) {
                                    //exception handling left as an exercise for the reader
                                }
                            }
                            str = str + "\n";
                              System.out.println(str);
                              /////
                              
                              
                              /*try (Writer writer = new BufferedWriter(new OutputStreamWriter(
                                            new FileOutputStream("data.txt"), "utf-8"))) {
                                 writer.write(str);
                              }*/
                              /////
                              
                        }
                          
			//pw.println("Hello, " + str);
			//pw.close();
                } 
                catch (Exception e) 
                {
                    System.out.println("ERROR STATUS");
                }
            }
        }).start();
    }
                     
    private void initComponents() 
    {

        jLabel1 = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        jSlider1 = new javax.swing.JSlider();
        jButton1 = new javax.swing.JButton();
        jButton2 = new javax.swing.JButton();
        jLabel2 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setPreferredSize(new java.awt.Dimension(250, 250));
        setResizable(false);
        addWindowListener(new java.awt.event.WindowAdapter() 
        {
            public void windowClosing(java.awt.event.WindowEvent evt) 
            {
                formWindowClosing(evt);
            }
        });

        jLabel1.setFocusable(false);
        jLabel1.setText("Internet of Things");
        getContentPane().add(jLabel1, java.awt.BorderLayout.PAGE_END);

        jPanel1.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        jPanel1.setLayout(null);
        
        jSlider1.setFocusable(false);
        jSlider1.setMinimum(0);
        jSlider1.setMaximum(180);
        jSlider1.setValue(90);
        jSlider1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)), "Set Servo Angle"));
        jSlider1.addChangeListener(new javax.swing.event.ChangeListener() 
        {
            public void stateChanged(javax.swing.event.ChangeEvent evt) 
            {
                int angle = jSlider1.getValue();
                String command = "SetAngle "+Integer.toString(angle)+"\n";
                
                try
                {
                    out.write(command.getBytes());
                    Thread.sleep(100);
                }
                catch(Exception e)
                {
                    System.out.println("NOT SEND!");
                    System.out.println(e);
                }
               
                jLabel2.setText(Integer.toString(angle)+"\u00b0");
                System.out.println(command);                
            }
        });
        
        jPanel1.add(jSlider1);
        jSlider1.setBounds(10, 20, 220, 60);

        jButton1.setFocusable(false);
        jButton1.setText("CLOSE TCP");
        jButton1.addActionListener(new java.awt.event.ActionListener() 
        {
            public void actionPerformed(java.awt.event.ActionEvent evt) 
            {
                String command = "CloseConnection\n";
                
                try
                {
                    out.write(command.getBytes());
                    out.close();
                    server.close();
                    serverSocket.close();
                    Thread.sleep(100);
                }
                catch(Exception e)
                {
                    System.out.println("NOT SEND!");
                    System.out.println(e);
                }
            }
        });
        jPanel1.add(jButton1);
        jButton1.setBounds(10, 90, 108, 40);

        jButton2.setFocusable(false);
        jButton2.setText("IoT(!)");
        jButton2.addActionListener(new java.awt.event.ActionListener() 
        {
            public void actionPerformed(java.awt.event.ActionEvent evt) 
            {
                OpenServer();
                jSlider1.setValue(90);
            }
        });
        jPanel1.add(jButton2);
        jButton2.setBounds(124, 90, 108, 40);
        
        jLabel2.setFocusable(false);
        jLabel2.setFont(new java.awt.Font("Courier New", 0, 36)); // NOI18N
        jLabel2.setText(Integer.toString(90)+"\u00b0");
        jLabel2.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)), "Servo Angle"));
        jLabel2.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jPanel1.add(jLabel2);
        jLabel2.setBounds(10, 130, 100, 60);

        getContentPane().add(jPanel1, java.awt.BorderLayout.CENTER);

        pack();
    }
    
    private void formWindowClosing(java.awt.event.WindowEvent evt) 
    {            
        String command = "CloseConnection\n";
                
        try
        {
            out.write(command.getBytes());
            out.close();
            server.close();
            serverSocket.close();
            Thread.sleep(100);
        }
        catch(Exception e)
        {
            System.out.println("NOT SEND!");
            System.out.println(e);
        }
        
        System.exit(0);
    } 
    
    // Variables declaration - do not modify                     
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton2;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JSlider jSlider1;
    // End of variables declaration                   
}
