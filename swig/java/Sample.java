import simstring.*;

public class Sample {
    static {
        try {
            System.loadLibrary("SimString");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Couldn't find the SimString library.");
        }
    }

    private static void sampleWriter() {
        // Create a SimString database with two person names.
        writer db = new writer("sample.db", 3, false, false);
        db.insert("Barack Hussein Obama II");
        db.insert("James Gordon Brown");
        db.close();
    }

    private static void output(StringVector strs) {
        // Output the retrieved strings separated by ", ".
        for (int i = 0;i < strs.size();++i) {
            if (i != 0) {
                System.out.print(", ");
            }
            System.out.print(strs.get(i));
        }
        System.out.print('\n');
    }

    private static void sampleReader() {
        // Open the database for reading.
        reader db = new reader("sample.db");

        // Use cosine similarity and threshold 0.6.
        db.setMeasure(simstringConstants.cosine);
        db.setThreshold(0.6);
        output(db.retrieve("Barack Obama"));
        output(db.retrieve("Gordon Brown"));
        output(db.retrieve("Obama"));

        // Use overlap coefficient and threshold 1.0.
        db.setMeasure(simstringConstants.overlap);
        db.setThreshold(1.);
        output(db.retrieve("Obama"));
    }

    public static void main(String[] args) {
        sampleWriter();
        sampleReader();
    }
}

