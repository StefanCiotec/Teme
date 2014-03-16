# Ciotec Marian-Stefan 323CA
#Tema 2
JFLAGS = -g
JC = javac
JVM= java 

.SUFFIXES: .java .class

CLASSES = \
	Bot.java \
	Coord.java \
	Pitch.java \
	Pair.java \
	Position.java \
	Status.java


MAIN = Bot


build:
	$(JC) $(JFLAGS) *.java

classes:
	$(CLASSES:.java=.class)
	
clean:
	$(RM) *.class



