import simstring.*;

public class Sample {

	private static void sampleWriter(){
		writer db = new writer("sample.db");
		db.insert("Barack Hussein Obama II");
		db.insert("James Gordon Brown");
		db.close();
	}
	private static void sampleReader(){
		reader db = new reader("sample.db");
		db.setMeasure(2);
		db.setThreshold(0.6);
		System.out.println(db.retrieve("Barack Obama").size());
		System.out.println(db.retrieve("Gordon Brown").size());
		System.out.println(db.retrieve("Obama").size());
		db.setMeasure(4);
		db.setThreshold(1);
		System.out.println(db.retrieve("Obama").size());
	}
	public static void main(String[] args){
		try { System.loadLibrary("SimString"); }
		catch (UnsatisfiedLinkError e) {
			System.out.println("Couldn't find the SimString library.");
		}
		sampleWriter();
		sampleReader();
	}
}
