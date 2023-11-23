#[[
    @brief  Download dependency library from git repository.
    @param  url - git repository address
    @param  branch - the branch or tag name
    @param  lib_name - the expected local name
]]
function(pipe_download_dependency url branch lib_name work_directory)
    execute_process(COMMAND bash "-c" "if [ ! -d ${work_directory} ];\
        then mkdir -p ${work_directory}; fi")

    execute_process(COMMAND bash "-c" "if [ ! -d ${lib_name} ]; then \
        git clone --progress -q --depth=1 -b ${branch} ${url} ${lib_name}; fi"
        WORKING_DIRECTORY ${work_directory})
endfunction()