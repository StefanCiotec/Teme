
import java.util.ArrayList;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author stefan
 */
public class Position {
    
    /*true daca se poate muta in acea direcite*/
    private boolean[] direction;
    /*true daca pozitia este margine*/
    private boolean margin;
    /*true daca face parte din poarta*/
    private boolean goal_post;
    public boolean SWCorner;
    public boolean SECorner;
    public boolean NWCorner;
    public boolean NECorner;
    
    public Position() {
        direction = new boolean[8];
        /*marchez toate cele 8 directii ca fiind accesibile*/
        for(int i = 0; i < 8; i++) {
            direction[i] = true;
        }
  
        margin = false;
        goal_post = false;
        SWCorner = false;
        SECorner = false;
        NWCorner = false;
        NECorner = false;
    }
    /*daca s-a mers in cel putin o directie
    * atunci prin acel punct s-a mai trecut
    */
    public boolean isMarked() {
        for(int i = 0; i < 8; i++) {
            if(!accessable(i)) {
                return true;
            }
        }
        return false;
    }
    /*checks if is margin*/
    public boolean isMargin() {
        return margin;
    }
    
    /*sets as margin*/
    public void setAsMargin() {
        margin = true;
    }
    
    public void setAsGoal() {
        goal_post = true;
    }
   
    
    /*checks if direction d is accesssable*/
    public boolean accessable(int d) {
        return direction[d];
    }
    
    public void markDirection(int d, boolean how) {
        /*how == false => direction d marked as not accessible
         * how == true => direction d marked as accessible
         */
        direction[d] = how;
        
    }
    
    /*intoarce toate directiile pe care se pot face mutari valide*/
    public ArrayList<Integer> getAccDirections() {
        ArrayList<Integer> solution = new ArrayList();
        for(int i = 0; i < 8; i++) {
            if(accessable(i)) {
                solution.add(i);
            }
        }
        return solution;
    }
    /*intoarce true daca trebuie sa mut eu*/
    public boolean makeAnotherMove() {
        if(SWCorner || SECorner || NWCorner || NECorner) {
            ArrayList<Integer> sol = getAccDirections();
            if(sol.size() == 7) {
                return false;
            }
            return true;
        }
        return ((isMarked() || isMargin()) && !goal_post);
    }
    
}
