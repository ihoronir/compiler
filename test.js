#!/usr/bin/env -S deno run --allow-run --allow-read --allow-write

const status = Object.freeze({
    compileErr: 'Compile Error',
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
    args: ['-C', './test/lib']
})).spawn().status;

if (!makelib.success) Deno.exit(1);

// [no, Promise] の配列
const results = [];

for await (const entry of Deno.readDir('./test')) {
    if (!entry.isFile) continue;

    const info = entry.name.match(/(.*)_([0-9]*).c$/);
    if (info  == null) continue;

    const [name, no, expected] = info;

    const run = async function() {
        const compiler = './target/compiler';
        const src      = './test/' + entry.name;
        const asm      = './test/asm/' + no + '.s';
        const target   = './test/target/' + no;

        const compileLog = './test/log/' + no + '_compile.log';
        const linkLog    = './test/log/' + no + '_link.log';
        const execLog    = './test/log/' + no + '_exec.log';

        // コンパイル
        const compile = await (new Deno.Command(compiler, {
            args: [src, asm], stderr: 'piped'
        })).spawn();

        if (!(await compile.status).success) {
            redirectToFile(compile.stderr, compileLog);
            return status.compileErr;
        } 

        // リンク
        const link = await (new Deno.Command('/bin/gcc', {
            args: ['-o', target, asm], stderr: 'piped'
        })).spawn();

        if (!(await link.status).success) {
            redirectToFile(link.stderr, linkLog);
            return status.linkErr;
        }

        // 実行
        const exec = await (new Deno.Command(target, {
            stderr: 'piped'
        })).spawn();

        if ((await exec.status).code != parseInt(expected)) {
            redirectToFile(exec.stderr, execLog);
            return status.execErr;
        }

        return status.success;
    }

    results.push([no, run()]);
}

for (const [no, resultPromise] of results.sort()) {
    const result = await resultPromise;
    switch (result) {
        case status.success:
            console.log(no, '\u001b[32m' + result + '\u001b[0m');
            break;

        case status.compileErr:
            console.log(no, '\u001b[33m' + result + '\u001b[0m');
            break;

        case status.linkErr:
            console.log(no, '\u001b[34m' + result + '\u001b[0m');
            break;

        case status.execErr:
            console.log(no, '\u001b[31m' + result + '\u001b[0m');
            break;
    }
};
