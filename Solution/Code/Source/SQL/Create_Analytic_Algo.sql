-- Analytic Algorithms Identification
USE [ATS_DEV]
GO

/****** Object:  Table [dbo].[analytic_algo]    Script Date: 4/4/2020 11:21:44 AM ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[analytic_algo](
	[uid] [int] NOT NULL,
	[algo_desc] [nvarchar](50) NOT NULL,
	CONSTRAINT [PK_analytic_algo] PRIMARY KEY CLUSTERED
		(
			[uid] ASC
		)	
		WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO

