#!/usr/bin/env -S deno run --allow-run --allow-read --allow-write

let allSuccess = true;

/**
 * path の末尾にはスラッシュをつけないでください
 */
async function test(path) {
    const status = Object.freeze({
        compileErr: 'Compile Error',
        compilerSegV: 'Compile SegFault',
        linkErr: 'Link Error',
        execErr: 'Exec Error',
        success: 'Success',
    });

    async function redirectToFile(src, filename) {
        src.pipeTo((await Deno.open(filename, {
            read: false, write: true, truncate: true, create: true
        })).writable);
    }

    // テスト用ライブラリのビルド

    const makelib = await (new Deno.Command('make', {
        args: ['-C', path + '/lib']
    })).spawn().status;

    if (!makelib.success) Deno.exit(1);

    // [no, Promise] の配列
    const results = [];

    for await (const entry of Deno.readDir(path + '')) {
        if (!entry.isFile) continue;

        const info = entry.name.match(/(.*)_([0-9]*)(_withlib)?.c$/);
        if (info == null) continue;

        const [name, no, expected, withlib] = info;

        const run = async function() {
            const compiler = './target/compiler';
            const src      = path + '/' + entry.name;
            const asm      = path + '/asm/' + no + '.s';
            const target   = path + '/target/' + no;

            const compileLog = path + '/log/' + no + '_compile.log';
            const linkLog    = path + '/log/' + no + '_link.log';
            const execLog    = path + '/log/' + no + '_exec.log';

            // コンパイル
            const compile = await (new Deno.Command(compiler, {
                args: [src, asm], stderr: 'piped'
            })).spawn();

            const compileStatus = (await compile.status);

            if (!compileStatus.success) {
                console.log({ reason: 'compile error', name: entry.name, compileStatus: compileStatus.code });
                redirectToFile(compile.stderr, compileLog);
                if (compileStatus.code == 139) {
                    return status.compilerSegV;
                }
                return status.compileErr;
            } 

            // リンク
            let linkargs;
            if (withlib) {
                linkargs = ['-o', target, asm, path + '/lib/lib.s', '-no-pie', '-Wno-unused-command-line-argument']

            } else {
                linkargs = ['-o', target, asm]
            }

            const link = await (new Deno.Command('/bin/gcc', {
                args: linkargs,
                stderr: 'piped'
            })).spawn();

            if (!(await link.status).success) {
                redirectToFile(link.stderr, linkLog);
                return status.linkErr;
            }

            // 実行
            const exec = await (new Deno.Command(target, {
                stderr: 'piped'
            })).spawn();

            const code = (await exec.status).code;
            if (code != parseInt(expected)) {
                console.log({ name: entry.name, code, expected });
                redirectToFile(exec.stderr, execLog);
                return status.execErr;
            }

            return status.success;
        }

        results.push([no, run(), entry.name]);
    }

    for (const [no, resultPromise, name] of results.sort()) {
        const result = await resultPromise;
        switch (result) {
            case status.success:
                console.log(no, '\u001b[32m' + result + '\u001b[0m');
                //await Deno.rename("./test/" + name, "./test_working/" + name);
                break;

            case status.compileErr:
                console.log(no, '\u001b[33m' + result + '\u001b[0m');
                //await Deno.rename("./test/" + name, "./test_todo/" + name);
                allSuccess = false;
                break;

            case status.compilerSegV:
                console.log(no, '\u001b[35m' + result + '\u001b[0m');
                //await Deno.rename("./test/" + name, "./test_todo/" + name);
                allSuccess = false;
                break;

            case status.linkErr:
                console.log(no, '\u001b[34m' + result + '\u001b[0m');
                //await Deno.rename("./test/" + name, "./test_todo/" + name);
                allSuccess = false;
                break;

            case status.execErr:
                console.log(no, '\u001b[31m' + result + '\u001b[0m');
                //await Deno.rename("./test/" + name, "./test_todo/" + name);
                allSuccess = false;
                break;
        }
    };
}

await test(Deno.args[0]);

Deno.exit(allSuccess ? 0 : 1);
