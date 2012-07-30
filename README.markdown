# About MasterCal
The UIowa Master Calendar is a centralized Calendar Service purchased by the 
University. This module leverages the Master Calendar API to pull in event data
as nodes.

## Installation
1. Download required modules: [Web Services Client](http://drupal.org/project/wsclient "Web Services Client module page"), [Entity API](http://drupal.org/project/entity "Entity API download page"), And make sure the PHP SOAP extension is enabled on your server.
2. Download the module to your preferred directory.
	* /sites/all/modules
	* sites/all/modules/custom
	* or sites/example.com/modules
3. Enable the module (It can be found under "UIowa Module Package" in the modules list).
4. Configure the module.
  * Go to "UIowa Master Calendar" under "UIowa Module Package" on the administration page.
  * Add a calendar. Once added, a calendar must be connected.
  * Edit the calendar. Each calendar requires it's own API username and password. Contact the UIowa Master Calendar team for credentials.
  * Once connected, additional fields can be configured on the edit calendar form. 
6. Run cron.

### Workbench Access
The mastercal module can integrate with Workbench Access. To do so follow these
steps.

1. Download and enable Workbench and Workbench Access. Follow their instructions
for installation and configuring section assignment.
2. Once enabled, additional configuration options will be available in the 
mastercal configuration page. Each calendar can select the taxonomy terms or 
menu items that should be assigned to those events.
3. By default, Retroactive Section Assignment is enabled on each calendar. This
assigns sections to existing events during cron. It can be turned off to decrease
cron running time. However, events will have to be manually updated if sections
assignment changes.

## Troubleshooting
Contact ITS Web Services.