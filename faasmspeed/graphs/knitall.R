#!/usr/bin/env Rscript
library(rmarkdown)
library(xfun)

tables = c('wtable', 'subtable', 'pcatable', 'thumbtable')

for (table in tables) {
    xfun::Rscript_call(
        rmarkdown::render,
        list(input = 'faasmspeed-param.Rmd',
            output_format = 'html_document',
            output_file = paste0('faasmspeed-', table, '.html'),
            params = list(input_path = paste0(table, '.csv'))
        )
    )
}
