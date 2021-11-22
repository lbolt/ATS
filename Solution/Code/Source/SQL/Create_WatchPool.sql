-- Reference ATS_CONST_CODES.h for fixed symbol sizes

USE [ATS_DEV]
GO



/****** Object:  Table [dbo].[watchpool]    Script Date: 12/11/2019 4:58:06 PM ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[watchpool](
	[sequence] [int] NOT NULL,
	[symbol] [nchar](8) NOT NULL,
	[avg_vol] [int] NOT NULL,
	[watch_eligible] [bit] NOT NULL,
	[trade_eligible] [bit] NOT NULL,
	[create_dt] [datetime2](7) NOT NULL,
	[update_dt] [datetime2](7) NOT NULL,
	[exchange] [nchar](12) NULL,
	[desc] [nchar](40) NULL,
	CONSTRAINT [PK_watchpool] PRIMARY KEY CLUSTERED 
		(
			[symbol] ASC
		)
		WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY],
	CONSTRAINT [AK_Sequence] UNIQUE NONCLUSTERED 
		(
			[sequence] ASC
		)
		WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

