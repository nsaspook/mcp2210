/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package cube;

import java.io.File;  // Import the File class
import java.io.FileNotFoundException;  // Import this class to handle errors
import java.util.Scanner;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.swing.JFrame;
import javax.vecmath.Quat4d;
import javax.vecmath.Vector3d;
import com.fazecast.jSerialComm.SerialPort;
import com.sun.j3d.utils.geometry.ColorCube;
import com.sun.j3d.utils.universe.SimpleUniverse;

/**
 *
 * @author root
 */
public class Cube {

    public static void main(String[] args) {

        JFrame frame = new JFrame("Sensor Fusion Visual Test Program");
        Canvas3D canvas = new Canvas3D(SimpleUniverse.getPreferredConfiguration());
        SimpleUniverse universe = new SimpleUniverse(canvas);
        BranchGroup group = new BranchGroup();
        ColorCube cube = new ColorCube(0.3);

        TransformGroup transformGroup = new TransformGroup();
        transformGroup.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
        transformGroup.addChild(cube);

        universe.getViewingPlatform().setNominalViewingTransform();
        group.addChild(transformGroup);
        universe.addBranchGraph(group);

        frame.add(canvas);
        frame.setSize(800, 600);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setVisible(true);

        SerialPort port = SerialPort.getCommPort("ttyACM0");
        port.setBaudRate(115200);
        port.setComPortTimeouts(SerialPort.TIMEOUT_SCANNER, 1, 1);
        if (port.openPort() == false) {
            System.err.println("Unable to open the serial port. Exiting.");
//            System.exit(1);
        }

        // Connect to the named pipe
        File myfifo = new File("/tmp/myfifo");
        try {
            Scanner myReader = new Scanner(myfifo);
            while (myReader.hasNextLine()) {
                String line = myReader.nextLine();
                String[] token = line.split(",");
                double w = Double.parseDouble(token[0]);
                double x = -Double.parseDouble(token[6]);
                double y = -Double.parseDouble(token[7]);
                double z = -Double.parseDouble(token[8]);
                double ax = Double.parseDouble(token[3]);
                double ay = Double.parseDouble(token[4]);
                double az = Double.parseDouble(token[5]);
                System.out.println(String.format("w = %+7.3f     x = %+7.3f     y = %+7.3f     z = %+7.3f", w, x, y, z));
                Quat4d quaternion = new Quat4d(w, x, y, z);
                Vector3d vector = new Vector3d((ax * 0.02), (ay * 0.02), (az * 0.02));
                transformGroup.setTransform(new Transform3D(quaternion, vector, 1.0));
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    //       Scanner s = new Scanner(port.getInputStream());
//        while (s.hasNextLine()) {
//            try {
//                String line = s.nextLine();
//                String[] token = line.split(" ");
    // multiply x/y/z by -1 to swap frames of reference
//                double w = Double.parseDouble(token[0]);
//                double x = -Double.parseDouble(token[1]);
//                double y = -Double.parseDouble(token[2]);
//                double z = -Double.parseDouble(token[3]);
//                double ax = Double.parseDouble(token[4]);
//                double ay = Double.parseDouble(token[5]);
//                double az = Double.parseDouble(token[6]);
    /*
                System.out.println(String.format("w = %+2.3f     x = %+2.3f     y = %+2.3f     z = %+2.3f", w, x, y, z));
     */
//                Quat4d quaternion = new Quat4d(w, x, y, z);
//                Vector3d vector = new Vector3d((az * 0.02), (ay * 0.02), (az * 0.02));
//                transformGroup.setTransform(new Transform3D(quaternion, vector, 1.0));
    // the inverse cosine of w gives you the pitch *if* you normalize the quaternion with x and z being zero
//                double pitch = Math.acos(w / Math.sqrt(w * w + y * y)) * 2.0 - (Math.PI / 2.0);
//                System.out.println(String.format("w = %+2.3f     x = %+2.3f     y = %+2.3f     z = %+2.3f     pitch = %+1.3f", w, x, y, z, pitch));
//            } catch (Exception e) {
//            }
//        }
//        s.close();
//        System.err.println("Lost communication with the serial port. Exiting.");
//        System.exit(1);
}

//}
