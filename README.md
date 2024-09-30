# monotile

## Setup

```
sudo apt-get install libcurl3-dev libsqlite3-dev
```

If using Apache,

```
sudo apt-get install apache2
make install
```

I'm probably going to turn this into a weird pixel thing.  But, for now... It's mostly a test to see just how simple it can be to use github identities.

If setting this up as a new repo, will need auth app here: https://github.com/apps/monotileapp

Based on https://docs.github.com/en/developers/apps/building-oauth-apps/authorizing-oauth-apps

## server setup

Make a file called `data/.private` in the root of this.  It should contain the following, depending on the oauth app you've created.  Note that you will need to make a new secret, and that will need to go here.  This text is urlencoded.

client_id=XXXXXXXXXXXXX&client_secret=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

