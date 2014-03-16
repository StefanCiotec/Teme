//CIOTEC MARIAN-STEFAN 333CA

//variabila globala care retine starea fisierelor
public class GlobalVars {
	//starea fisierelor
	public static FileStatistics[] file_stat;
	//lock[i] - semafor pentru actualizarea file_stat[i]
	public static Object [] lock;
	//vectorul de delimitatori
	public static final char[] delims = new char[]{'\t','\n','\r','\f','.', ',', '?', '!',':',' ', '(', ')'};
	//vector care retine similaritatile intre documente
	//sim[i] retine similitudinea intre fisierul de verificat pentru plagiat si fisierul i
	public static double [] sim;
}
