public class Pair {

	public int first, second;

	public Pair(int first, int second) {
		this.first = first;
		this.second = second;
	}
        @Override
	public String toString() {
		return "(" + first + "," + second + ")";
	}

}
