all : app/authback.cgi app/logout.cgi

app/authback.cgi : src/authback.c
	gcc -o $@ $^ -Os -s -lcurl -lsqlite3

app/logout.cgi : src/logout.c
	gcc -o $@ $^ -Os -s -lsqlite3

createdb :
	echo "CREATE TABLE IF NOT EXISTS logins( key TEXT PRIMARY KEY, name TEXT NOT NULL, logintime DATE )" | sqlite3 monotile.db

clean :
	rm -rf app/authback.cgi app/logout.cgi
