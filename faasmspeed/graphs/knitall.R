library(rmarkdown)
library(xfun)

xfun::Rscript_call(
    rmarkdown::render,
    list(input = 'faasmspeed-param.Rmd',
        output_format = 'html_document',
        output_file = 'faasmspeed-wiki4M.html',
        params = list(input_path = 'wtable.csv')
    )
)
