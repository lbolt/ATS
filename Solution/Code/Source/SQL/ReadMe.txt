
SQL (SQL Scripts)
	- CreateWatchPool_DDL.sql		



SPI (Stored Procedure)


SQL Tables:
	- watchpool:					securities eligible for watching and analysis (static)
	- position_rec:					records that are eligable for trading (10 being most and 1 least)
	- archived_position_rec:		expired position records
	- blotter_trade_rec:			active position
	- blotter_trade_data:			data record to support active position (each trade will be composed of one or more segments)
	- blotter_archived_trade_rec:	closed position archive (data segments are colapsed into a single record with volume and amounts rolled-up into a single trade)
