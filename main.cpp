#include "AhoCorasick.h"
#include <iostream>
#include <fstream>
#include <chrono>

// 生成2000条模拟的恶意特征规则
std::vector<std::string> generateMaliciousRules(int count) {
    std::vector<std::string> rules;
    
    // 基础恶意特征
    std::vector<std::string> bases = {
        "her.exe", "rundll32", "powershell -e", "cmd.exe /c",
        "wscript.exe", "cscript.exe", "regsvr32", "mshta.exe",
        "certutil.exe", "bitsadmin.exe", "schtasks.exe", "at.exe",
        "nslookup.exe", "ipconfig.exe", "ping.exe", "tracert.exe",
        "net.exe", "net1.exe", "netstat.exe", "tasklist.exe",
        "taskkill.exe", "attrib.exe", "del.exe", "erase.exe",
        "move.exe", "copy.exe", "xcopy.exe", "robocopy.exe",
        "ren.exe", "rename.exe", "mkdir.exe", "md.exe",
        "rmdir.exe", "rd.exe", "cd.exe", "chdir.exe",
        "cls.exe", "exit.exe", "echo.exe", "type.exe",
        "find.exe", "findstr.exe", "sort.exe", "more.exe",
        "less.exe", "head.exe", "tail.exe", "wc.exe",
        "grep.exe", "sed.exe", "awk.exe", "perl.exe",
        "python.exe", "ruby.exe", "php.exe", "java.exe",
        "javaw.exe", "msbuild.exe", "csc.exe", "vbc.exe",
        "link.exe", "cl.exe", "nmake.exe", "make.exe",
        "cmake.exe", "git.exe", "svn.exe", "hg.exe",
        "ftp.exe", "tftp.exe", "telnet.exe", "ssh.exe",
        "sftp.exe", "scp.exe", "rsync.exe", "wget.exe",
        "curl.exe", "aria2c.exe", "idm.exe", "flashget.exe",
        "thunder.exe", "emule.exe", "bittorrent.exe", "utorrent.exe",
        "qbittorrent.exe", "deluge.exe", "transmission.exe", "vuze.exe",
        "skype.exe", "teams.exe", "zoom.exe", "wechat.exe",
        "qq.exe", "alibaba.exe", "taobao.exe", "tmall.exe",
        "jd.exe", "pinduoduo.exe", "meituan.exe", "ele.me.exe",
        "didi.exe", "uber.exe", "lyft.exe", "airbnb.exe",
        "booking.exe", "expedia.exe", "tripadvisor.exe", "ctrip.exe",
        "qunar.exe", "elong.exe", "tuniu.exe", "lvmama.exe",
        "cctv.exe", "xinhua.exe", "people.exe", "china.com.exe",
        "sina.exe", "sohu.exe", "163.exe", "qq.com.exe",
        "baidu.exe", "google.exe", "bing.exe", "yahoo.exe",
        "msn.exe", "aol.exe", "lycos.exe", "altavista.exe",
        "ask.com.exe", "excite.exe", "infoseek.exe", "webcrawler.exe",
        "dogpile.exe", "metacrawler.exe", "mamma.com.exe", "search.com.exe",
        "findwhat.com.exe", "looksmart.com.exe", "about.com.exe", "howstuffworks.com.exe",
        "wikipedia.org.exe", "wikimedia.org.exe", "commons.wikimedia.org.exe", "en.wikipedia.org.exe",
        "zh.wikipedia.org.exe", "ja.wikipedia.org.exe", "fr.wikipedia.org.exe", "de.wikipedia.org.exe",
        "es.wikipedia.org.exe", "ru.wikipedia.org.exe", "it.wikipedia.org.exe", "pt.wikipedia.org.exe",
        "nl.wikipedia.org.exe", "sv.wikipedia.org.exe", "pl.wikipedia.org.exe", "tr.wikipedia.org.exe",
        "ar.wikipedia.org.exe", "hi.wikipedia.org.exe", "bn.wikipedia.org.exe", "pa.wikipedia.org.exe",
        "gu.wikipedia.org.exe", "or.wikipedia.org.exe", "ta.wikipedia.org.exe", "te.wikipedia.org.exe",
        "kn.wikipedia.org.exe", "ml.wikipedia.org.exe", "si.wikipedia.org.exe", "th.wikipedia.org.exe",
        "vi.wikipedia.org.exe", "id.wikipedia.org.exe", "ms.wikipedia.org.exe", "km.wikipedia.org.exe",
        "lo.wikipedia.org.exe", "my.wikipedia.org.exe", "zh-yue.wikipedia.org.exe", "zh-min-nan.wikipedia.org.exe",
        "zh-classical.wikipedia.org.exe", "wiktionary.org.exe", "wikibooks.org.exe", "wikiversity.org.exe",
        "wikinews.org.exe", "wikivoyage.org.exe", "wikiquote.org.exe", "wikisource.org.exe",
        "wikimediafoundation.org.exe", "donate.wikimedia.org.exe", "foundation.wikimedia.org.exe", "wikimediaendowment.org.exe",
        "wikimediapartners.org.exe", "wikimediapressroom.org.exe", "wikimediaprojects.org.exe", "wikimediatech.org.exe",
        "wikimediathanks.org.exe", "wikimediayearinreview.org.exe", "wikispecies.org.exe", "mediawiki.org.exe",
        "www.mediawiki.org.exe", "meta.wikimedia.org.exe", "incubator.wikimedia.org.exe", "test.wikipedia.org.exe",
        "test2.wikipedia.org.exe", "beta.wikimedia.org.exe", "labtest.wikimedia.org.exe", "tools.wikimedia.org.exe",
        "toolforge.org.exe", "wdqs.toolforge.org.exe", "query.wikidata.org.exe", "www.wikidata.org.exe",
        "www.wikimedia.org.exe", "www.wikipedia.org.exe", "www.wiktionary.org.exe", "www.wikibooks.org.exe",
        "www.wikiversity.org.exe", "www.wikinews.org.exe", "www.wikivoyage.org.exe", "www.wikiquote.org.exe",
        "www.wikisource.org.exe", "www.wikispecies.org.exe", "www.mediawiki.org.exe", "www.meta.wikimedia.org.exe",
        "www.incubator.wikimedia.org.exe", "www.test.wikipedia.org.exe", "www.test2.wikipedia.org.exe", "www.beta.wikimedia.org.exe",
        "www.labtest.wikimedia.org.exe", "www.tools.wikimedia.org.exe", "www.toolforge.org.exe", "www.wdqs.toolforge.org.exe",
        "www.query.wikidata.org.exe", "www.wikidata.org.exe", "www.wikimediafoundation.org.exe", "www.donate.wikimedia.org.exe",
        "www.foundation.wikimedia.org.exe", "www.wikimediaendowment.org.exe", "www.wikimediapartners.org.exe", "www.wikimediapressroom.org.exe",
        "www.wikimediaprojects.org.exe", "www.wikimediatech.org.exe", "www.wikimediathanks.org.exe", "www.wikimediayearinreview.org.exe"
    };
    
    // 生成规则
    for (int i = 0; i < count; ++i) {
        std::string rule = bases[i % bases.size()];
        // 为了增加规则的多样性，随机添加一些前缀或后缀
        if (i % 3 == 0) {
            rule = "./" + rule;
        } else if (i % 3 == 1) {
            rule = rule + " -arg";
        } else {
            rule = "C:\\Windows\\System32\\" + rule;
        }
        rules.push_back(rule);
    }
    
    return rules;
}

int main() {
    std::cout << "=== Aho-Corasick IDS Rule Matching Engine ===\n";
    
    // 创建Aho-Corasick实例，忽略大小写
    AhoCorasick ac(true);
    
    // 生成2000条模拟的恶意特征规则
    std::cout << "1. Generating 2000 malicious rules...\n";
    auto rules = generateMaliciousRules(2000);
    
    // 添加所有规则到Aho-Corasick实例
    std::cout << "2. Adding rules to Aho-Corasick...\n";
    for (const auto& rule : rules) {
        ac.addRule(rule);
    }
    std::cout << "   Added " << ac.getRuleCount() << " rules.\n";
    
    // 构建失败转移指针和输出链表
    std::cout << "3. Building failure transitions and output links...\n";
    auto start_build = std::chrono::high_resolution_clock::now();
    ac.build();
    auto end_build = std::chrono::high_resolution_clock::now();
    auto duration_build = std::chrono::duration_cast<std::chrono::milliseconds>(end_build - start_build);
    std::cout << "   Build completed in " << duration_build.count() << " ms.\n";
    
    // 测试文本
    std::string test_text = 
        "The user executed C:\\Windows\\System32\\rundll32 with suspicious arguments. "
        "Also, there was an attempt to run powershell -e encodedcommand which is a known malicious pattern. "
        "Additionally, ./her.exe was found running from an unusual directory ./temp/her.exe.";
    
    // 匹配测试文本
    std::cout << "4. Matching test text...\n";
    auto start_match = std::chrono::high_resolution_clock::now();
    auto results = ac.match(test_text);
    auto end_match = std::chrono::high_resolution_clock::now();
    auto duration_match = std::chrono::duration_cast<std::chrono::microseconds>(end_match - start_match);
    
    // 输出匹配结果
    std::cout << "   Match completed in " << duration_match.count() << " us.\n";
    std::cout << "   Found " << results.size() << " matches:\n";
    for (const auto& result : results) {
        int rule_id = result.first;
        int start_pos = result.second;
        std::cout << "   - Rule ID: " << rule_id << ", Pattern: \"" << rules[rule_id] << "\", Start Position: " << start_pos << std::endl;
    }
    
    std::cout << "\n=== Engine test completed successfully! ===\n";
    
    return 0;
}