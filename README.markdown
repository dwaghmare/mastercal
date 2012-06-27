ABOUT MASTERCAL MASTER DEV BRANCH
=================================

The MasterCal product is a centralized Calendar Service purchased by the University. 
This module leverages the Master Calendar API to pull in event data as event type nodes.

INSTALLATION
------------

1. Download required modules: [Web Services Client](http://drupal.org/project/wsclient "Web Services Client module page"), [Entity API](http://drupal.org/project/entity "Entity API download page"), And make sure the PHP SOAP extension is enabled on your server.
2. Download the module to your preferred directory:
	- /sites/all/modules
	- sites/all/modules/custom
	- or sites/example.com/modules
3. Enable the module (It can be found under "UI Module Package" in the modules list)
4. Navigate to admin/config/user-interface/mastercal
5. Add one or more Calendars
6. Run Cron

CURRENT DEVELOPMENT
-------------------

Currently switching the module from nusoap to the Web Services Client module (which can be found [here](http://drupal.org/project/wsclient "Web Services Client module page"))

TROUBLESHOOTING
---------------

Email [Erin Corson](mailto:erin-corson@uiowa.edu "Mail Erin Corson")