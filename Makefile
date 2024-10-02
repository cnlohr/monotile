
TARGS := app/authback.cgi app/logout.cgi app/test.cgi app/test_federate.cgi app/plot.cgi app/stream.cgi
all : $(TARGS)

app/authback.cgi : src/authback.c
	gcc -o $@ $^ -Og -g  -lcurl -lsqlite3

app/test.cgi : src/test.c
	gcc -o $@ $^ -Og -g

app/test_federate.cgi : src/test_federate.c
	gcc -o $@ $^ -O2

app/logout.cgi : src/logout.c
	gcc -o $@ $^ -Og -g -lsqlite3

app/plot.cgi : src/plot.c
	gcc -o $@ $^ -Og -g -lsqlite3

app/stream.cgi : src/stream.c
	gcc -o $@ $^ -Og -g -lsqlite3


install : $(TARGS)
	test -f data/.privateappauth || echo "Error: must set a private key in privateappauth, in the form client_id=XXXXXXXXXXXXX&client_secret=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	sudo cp monotile.conf /etc/apache2/conf-available/
	sudo a2enconf monotile
	sudo service apache2 reload
	chmod 775 data
	touch data/log.txt
	chmod 666 data/log.txt

	mkdir -p data/database
	chmod 777 -R data/database
	echo "CREATE TABLE IF NOT EXISTS logins( lid INTEGER PRIMARY KEY AUTOINCREMENT, key TEXT, name TEXT NOT NULL, userurl TEXT, logintime DATE NOT NULL, valid BOOLEAN )" | sqlite3 data/database/monotile.db
	echo "CREATE TABLE IF NOT EXISTS users( uid INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE NOT NULL )" | sqlite3 data/database/monotile.db
	echo "CREATE TABLE IF NOT EXISTS pixels( pid INTEGER PRIMARY KEY AUTOINCREMENT, lid INTEGER NOT NULL, cx INTEGER NOT NULL, cy INTEGER NOT NULL, bx INTEGER NOT NULL, by INTEGER NOT NULL, dtime DATE NOT NULL, color INTEGER NOT NULL, removed BOOLEAN )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS key_to_lid on logins( key )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS username_to_uid on users( name )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS pix_cx on pixels( cx )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS pix_cy on pixels( cy )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS pix_bx on pixels( bx )" | sqlite3 data/database/monotile.db
	echo "CREATE INDEX IF NOT EXISTS pix_by on pixels( by )" | sqlite3 data/database/monotile.db
	chmod 666 data/database/monotile.db
	# Assume we work on a 2048x2048 grid for now.
	test -f data/grid.dat || dd if=/dev/zero of=data/grid.dat bs=2048 count=2048
	chmod 666 data/grid.dat

clean :
	rm -rf $(TARGS)
